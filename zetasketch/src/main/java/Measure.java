import com.google.zetasketch.*;
import java.lang.Math;
import java.util.ArrayList;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.BufferedInputStream;
import java.io.DataInputStream;

public class Measure {
    public static String[] readStrings(int n, int len) {
        long start = System.nanoTime();
        String[] arr = new String[n];
        byte[] buff = new byte[len];
        try {
            BufferedInputStream bin = new BufferedInputStream(System.in);
            for (int i = 0; i < n; ++i) {
                bin.read(buff, 0, len);
                arr[i] = new String(buff);
            }
        }
        catch (IOException io) {
            io.printStackTrace();
            System.exit(1);
        }
        long end = System.nanoTime();
        double seconds = (end - start) / 1e9;
        System.err.println("Reading data took " + Double.toString(seconds));
        return arr;
    }



    public static Long[] readLongs(int n) {
        long start = System.nanoTime();
        Long[] arr = new Long[n];
        for (int i = 0; i < n; ++i)
            arr[i] = 0L;
        try {
            BufferedInputStream bin = new BufferedInputStream(System.in);
            DataInputStream din = new DataInputStream(bin);
            for (int i = 0; i < n; ++i) {
                arr[i] = din.readLong();
            }
        }
        catch (IOException io) {
            io.printStackTrace();
            System.exit(1);
        }
        long end = System.nanoTime();
        double seconds = (end - start) / 1e9;
        System.err.println("Reading data took " + Double.toString(seconds));
        return arr;
    }


    
    public static void report(double seconds, double estimate,
                              int logM) {
        System.out.printf("time %g\n", seconds);
        System.out.printf("estimate %f\n", estimate);
        System.out.printf("bitsize %d\n", (1 << logM)*8);
        System.out.println("compressCount -1");
        System.out.println("rebaseCount -1");
    }


    
    public static <T> void measureQuery(HyperLogLogPlusPlus<T> hll,
                                        T[] data) {
        long start = System.nanoTime();
        for (T d : data)
            hll.add(d);
        long end = System.nanoTime();
        double seconds = (end - start) / 1e9;
        report(seconds, hll.result(), hll.getNormalPrecision());
    }

    
    
    public static <T> void measureMerge(HyperLogLogPlusPlus<T> hll1,
                                        HyperLogLogPlusPlus<T> hll2,
                                        HyperLogLogPlusPlus<T> hll3,
                                        T[] data) {
        int n1 = data.length / 2;
        for (int i = 0; i < n1; ++i)
            hll1.add(data[i]);
        for (int i = n1; i < data.length; ++i)
            hll2.add(data[i]);        
        
        long start = System.nanoTime();
        hll3.merge(hll1);
        hll3.merge(hll2);
        long end = System.nanoTime();
        double seconds = (end - start) / 1e9;
        report(seconds, hll3.result(), hll3.getNormalPrecision());
    }


    
    public static void measureQueryStrings(int logM, int n, int len) {
        String[] data = readStrings(n, len);
        HyperLogLogPlusPlus<String> hll = new HyperLogLogPlusPlus
            .Builder().normalPrecision(logM).noSparseMode()
            .buildForStrings();
        measureQuery(hll, data);
    }

    

    public static void measureQueryLongs(int logM, int n) {
        Long[] data = readLongs(n);
        HyperLogLogPlusPlus<Long> hll = new HyperLogLogPlusPlus
            .Builder().normalPrecision(logM).noSparseMode()
            .buildForLongs();
        measureQuery(hll, data);
    }


    
    public static void measureMergeStrings(int logM, int n, int len) {
        String[] data = readStrings(n, len);
        var builder = new HyperLogLogPlusPlus.Builder()
            .normalPrecision(logM).noSparseMode();
        HyperLogLogPlusPlus<String> hll1 = builder.buildForStrings();
        HyperLogLogPlusPlus<String> hll2 = builder.buildForStrings();
        HyperLogLogPlusPlus<String> hll3 = builder.buildForStrings();
        measureMerge(hll1, hll2, hll3, data);
    }

    

    public static void measureMergeLongs(int logM, int n) {
        Long[] data = readLongs(n);
        var builder = new HyperLogLogPlusPlus.Builder()
            .normalPrecision(logM).noSparseMode();
        HyperLogLogPlusPlus<Long> hll1 = builder.buildForLongs();
        HyperLogLogPlusPlus<Long> hll2 = builder.buildForLongs();
        HyperLogLogPlusPlus<Long> hll3 = builder.buildForLongs();
        measureMerge(hll1, hll2, hll3, data);
    }


    
    public static void measureQuery(String dt, int logM, int n, int len) {
        if ("str".equals(dt))
            measureQueryStrings(logM, n, len);
        else if ("uint64".equals(dt))
            measureQueryLongs(logM, n);
    }

    
    
    public static void measureMerge(String dt, int logM, int n, int len) {
        if ("str".equals(dt))
            measureMergeStrings(logM, n, len);
        else if ("uint64".equals(dt))
            measureMergeLongs(logM, n);
    }


    
    public static void main(String[] args) {
        if (args.length < 4 || args.length > 5) {
            System.err.println("usage: java -jar measure.jar <merge|query> <str|uint64> <m> <n> [len]");
            System.err.println("  where");
            System.err.println("merge - perform a merge experiment");
            System.err.println("query - perform a query experiment");
            System.err.println("str - assume stdin contains strings (one per line)");
            System.err.println("uint64 - assume stdin contains uint64s (one per line)");
            System.err.println("m - number of registers (a power of two)");
            System.err.println("n - the number of elements to read from stdin");
            System.err.println("len - length of strings to read from stdin");
            return;
        }

        String mode = args[0];
        if (!"merge".equals(mode) && !"query".equals(mode)) {
            System.err.println("Invalid mode `" + mode + "' given: must be `merge' or `query'");
            return;
        }
        String dt = args[1];
        if (!"str".equals(dt) && !"uint64".equals(dt)) {
            System.err.println("Invalid datatype `" + dt + "' given: must be `str' or `uint64'");
            return;
        }
        int m = Integer.parseInt(args[2]);
        int logM = (int)(Math.log(m)/Math.log(2));
        if (m != (1 << logM)) {
            System.err.println("The number of registers must be a power of two!");
            return;
        }

        int n = Integer.parseInt(args[3]);
        if (n < 0) {
            System.err.println("The number of elements to read must be non-negative!");
            return;
        }

        int len = 0;
        if ("str".equals(dt) && args.length != 5) {
            System.err.println("Len must be given for datatype str");
            return;
        }
        else if ("str".equals(dt) && args.length == 5) {
            len = Integer.parseInt(args[4]);
            if ("str".equals(dt) && len < 1) {
                System.err.println("Len must be positive");
                return;
            }
        }
        else if (!"str".equals(dt) && args.length == 5) {
            System.err.println("Len must be given only for datatype str");
            return;
        }

        if ("query".equals(mode))
            measureQuery(dt, logM, n, len);
        else if ("merge".equals(mode))
            measureMerge(dt, logM, n, len);        
    }
}
