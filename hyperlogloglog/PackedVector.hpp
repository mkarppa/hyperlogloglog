#ifndef HYPERLOGLOGLOG_PACKED_VECTOR
#define HYPERLOGLOGLOG_PACKED_VECTOR

#include <cstdint>
#include <climits>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cassert>
#include <cstring>

static_assert(CHAR_BIT == 8);

namespace hyperlogloglog {
  /**
   * This class represents a ``packed vector'', that is, a vector of
   * multibit (but constant size) elements that are stored in an array
   * of words such that one word can contain multiple elements, and
   * elements can cross word boundaries.
   *
   * This enable space-efficient but time-inefficient storage.
   */
  template<typename Word = uint64_t>
  class PackedVector {
  public:
    /**
     * elemSize : size of an individual element in bits
     * initialSize : how many zero elements to store in the array initially
     */
    explicit PackedVector(size_t elemSize, size_t initialSize = 0) :
      elemSize(elemSize),
      elemMask(~(~((Word)0) << elemSize)),
      size_(initialSize),
      capacity_((initialSize*elemSize +
                 WORD_BITS-1) / WORD_BITS)
    {
      if (capacity_ > 0) {
        arr = new Word[capacity_];
        memset(arr, 0, sizeof(Word)*capacity_);
      }
    }


    ~PackedVector() {
      delete[] arr;
    }



    friend void swap(PackedVector& a, PackedVector& b) {
      std::swap(a.elemSize, b.elemSize);
      std::swap(a.elemMask, b.elemMask);
      std::swap(a.size_, b.size_);
      std::swap(a.arr, b.arr);
      std::swap(a.capacity_, b.capacity_);
    }


    PackedVector(const PackedVector& that) :
      elemSize(that.elemSize),
      elemMask(that.elemMask),
      size_(that.size_),
      arr(new Word[that.capacity_]),
      capacity_(that.capacity_)
    {
      memcpy(arr, that.arr, sizeof(Word)*capacity_);
    }
    
    PackedVector(PackedVector&& that) {
      swap(*this, that);
    }



    PackedVector() {
    }


    
    PackedVector& operator=(const PackedVector& that) {
      if (this != &that) {
        PackedVector temp(that);
        swap(temp,*this);
      }
      return *this;
    }
    
    PackedVector& operator=(PackedVector&& that) {
      if (this != &that) {
        swap(*this, that);
      }
      return *this;
    }



    /**
     * Returns the number of bits stored in the vector (elemSize * size)
     */
    inline size_t bitSize() const {
      return size() * elemSize;
    }



    /**
     * Returns the ith element
     */
    Word get(size_t i) const {
      size_t firstBit = i*elemSize;      
      const Word* w = &arr[firstBit/WORD_BITS];
      firstBit %= WORD_BITS;
      if (firstBit + elemSize <= WORD_BITS) {
        return (*w >> firstBit) & elemMask;
      }
      else {
        size_t numBits = WORD_BITS-firstBit;
        Word e = *w >> firstBit;
        ++w;
        e |= (*w << numBits) & elemMask;
        return e;
      }
    }



    /**
     * Sets the ith element
     */
    void set(size_t i, Word e) {
      e &= elemMask;
      size_t firstBit = i*elemSize;
      Word* w = &arr[firstBit/WORD_BITS];
      firstBit %= WORD_BITS;
      if (firstBit + elemSize <= WORD_BITS) {
        *w &= ~(elemMask << firstBit);
        *w |= e << firstBit;
      }
      else {
        size_t numBits = WORD_BITS-firstBit;
        *w &= ~(elemMask << firstBit);
        *w |= e << firstBit;
        ++w;
        *w &= ~(elemMask >> numBits);
        *w |= e >> numBits;
      }
    }



    /**
     * Inserts the element at the ith position, and shifts all
     * remaining elements by one position. Potentially increases the
     * underlying array size.
     */
    void insert(size_t i, Word e) {
      append(0);
      for (int j = static_cast<int>(size()) - 2; j >= static_cast<int>(i); --j) 
        set(j+1,get(j));
      set(i,e);
    }



    /**
     * Removes the element at the ith position, and shifs all
     * remaining elements to the left by one position. Will not change
     * the underlying array size.
     */
    void erase(size_t i) {
      for (size_t j = i+1; j < size(); ++j)
        set(j-1,get(j));
      --size_;
    }
    

    
    /**
     * Returns the present size.
     */
    size_t size() const {
      return size_;
    }


    /**
     * Returns the number of elements that can be stored in the
     * present underlying array without reallocation.
     */
    size_t capacity() const {
      assert((elemSize == 0 && capacity_ == 0) ||
             elemSize > 0);
      return capacity_ > 0 ? capacity_*WORD_BITS/elemSize : 0;
    }



    /**
     * Add a new element to the end of the array, potentially
     * increasing its size.
     */
    void append(Word e) {
      size_t i = size_++;
      if (capacity_ == 0) {
        assert(arr == nullptr);
        arr = new Word[++capacity_];
        arr[0] = 0;        
      }
      else if (size_ * elemSize > WORD_BITS * capacity_) {
        /* increase array size */
        Word* newArr = new Word[capacity_+1];
        memcpy(newArr, arr, sizeof(Word)*capacity_);
        delete[] arr;
        arr = newArr;
        arr[capacity_] = 0;
        ++capacity_;
      }
      set(i,e);
    }

    

  private:
    static const size_t WORD_BITS = sizeof(Word)*CHAR_BIT;
    
    size_t elemSize = 0;
    Word elemMask = 0;    
    size_t size_ = 0; // number of logical bit elements stored in arr
    Word* arr = nullptr;
    size_t capacity_ = 0; // number of elements in arr
  };
}

#endif // HYPERLOGLOGLOG_PACKED_VECTOR
