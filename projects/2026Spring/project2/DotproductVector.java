import java.util.Locale;
import java.util.Random;

import jdk.incubator.vector.DoubleVector;
import jdk.incubator.vector.FloatVector;
import jdk.incubator.vector.VectorOperators;
import jdk.incubator.vector.VectorSpecies;

public final class DotproductVector {
    private static final int[] SIZES = new int[] {10, 100, 1_000, 10_000, 100_000, 1_000_000, 10_000_000, 100_000_000};

    private static final VectorSpecies<Float> FLOAT_SPECIES = FloatVector.SPECIES_PREFERRED;
    private static final VectorSpecies<Double> DOUBLE_SPECIES = DoubleVector.SPECIES_PREFERRED;

    private static volatile long checksumInt = 0;
    private static volatile double checksumFp = 0.0;

    private static int repsForN(int n) {
        if (n >= 100_000_000) return 1;
        if (n >= 10_000_000) return 2;

        long targetOps = 20_000_000L;
        long reps = targetOps / Math.max(1, n);
        if (reps < 3) reps = 3;
        if (reps > 2_000_000L) reps = 2_000_000L;
        return (int) reps;
    }

    private static void printHeader() {
        System.out.println("lang,type,N,reps,total_ns,avg_ns,checksum");
    }

    private static void fill(byte[] a, byte[] b, int n, long seed) {
        Random r = new Random(seed);
        for (int i = 0; i < n; i++) {
            a[i] = (byte) (r.nextInt(31) - 15);
            b[i] = (byte) (r.nextInt(31) - 15);
        }
    }

    private static void fill(short[] a, short[] b, int n, long seed) {
        Random r = new Random(seed);
        for (int i = 0; i < n; i++) {
            a[i] = (short) (r.nextInt(2001) - 1000);
            b[i] = (short) (r.nextInt(2001) - 1000);
        }
    }

    private static void fill(int[] a, int[] b, int n, long seed) {
        Random r = new Random(seed);
        for (int i = 0; i < n; i++) {
            a[i] = r.nextInt(2_000_001) - 1_000_000;
            b[i] = r.nextInt(2_000_001) - 1_000_000;
        }
    }

    private static void fill(float[] a, float[] b, int n, long seed) {
        Random r = new Random(seed);
        for (int i = 0; i < n; i++) {
            a[i] = (float) (r.nextDouble() * 2.0 - 1.0);
            b[i] = (float) (r.nextDouble() * 2.0 - 1.0);
        }
    }

    private static void fill(double[] a, double[] b, int n, long seed) {
        Random r = new Random(seed);
        for (int i = 0; i < n; i++) {
            a[i] = r.nextDouble() * 2.0 - 1.0;
            b[i] = r.nextDouble() * 2.0 - 1.0;
        }
    }

    private static long dotOnce(byte[] a, byte[] b, int n) {
        long acc = 0;
        for (int i = 0; i < n; i++) acc += (long) a[i] * (long) b[i];
        return acc;
    }

    private static long dotOnce(short[] a, short[] b, int n) {
        long acc = 0;
        for (int i = 0; i < n; i++) acc += (long) a[i] * (long) b[i];
        return acc;
    }

    private static long dotOnce(int[] a, int[] b, int n) {
        long acc = 0;
        for (int i = 0; i < n; i++) acc += (long) a[i] * (long) b[i];
        return acc;
    }

    private static double dotOnceVector(float[] a, float[] b, int n) {
        int i = 0;
        int upper = FLOAT_SPECIES.loopBound(n);
        FloatVector vecAcc = FloatVector.zero(FLOAT_SPECIES);

        for (; i < upper; i += FLOAT_SPECIES.length()) {
            FloatVector va = FloatVector.fromArray(FLOAT_SPECIES, a, i);
            FloatVector vb = FloatVector.fromArray(FLOAT_SPECIES, b, i);
            vecAcc = vecAcc.add(va.mul(vb));
        }

        double acc = vecAcc.reduceLanes(VectorOperators.ADD);
        for (; i < n; i++) acc += (double) a[i] * (double) b[i];
        return acc;
    }

    private static double dotOnceVector(double[] a, double[] b, int n) {
        int i = 0;
        int upper = DOUBLE_SPECIES.loopBound(n);
        DoubleVector vecAcc = DoubleVector.zero(DOUBLE_SPECIES);

        for (; i < upper; i += DOUBLE_SPECIES.length()) {
            DoubleVector va = DoubleVector.fromArray(DOUBLE_SPECIES, a, i);
            DoubleVector vb = DoubleVector.fromArray(DOUBLE_SPECIES, b, i);
            vecAcc = vecAcc.add(va.mul(vb));
        }

        double acc = vecAcc.reduceLanes(VectorOperators.ADD);
        for (; i < n; i++) acc += a[i] * b[i];
        return acc;
    }

    private static void warmupByte() {
        int n = 100_000;
        byte[] a = new byte[n];
        byte[] b = new byte[n];
        fill(a, b, n, 0xC0FFEE + n);
        for (int i = 0; i < 10; i++) checksumInt += dotOnce(a, b, n);
    }

    private static void warmupShort() {
        int n = 100_000;
        short[] a = new short[n];
        short[] b = new short[n];
        fill(a, b, n, 0xBADC0DE + n);
        for (int i = 0; i < 10; i++) checksumInt += dotOnce(a, b, n);
    }

    private static void warmupInt() {
        int n = 100_000;
        int[] a = new int[n];
        int[] b = new int[n];
        fill(a, b, n, 0x12345678L + n);
        for (int i = 0; i < 10; i++) checksumInt += dotOnce(a, b, n);
    }

    private static void warmupFloat() {
        int n = 100_000;
        float[] a = new float[n];
        float[] b = new float[n];
        fill(a, b, n, 0x31415926L + n);
        for (int i = 0; i < 10; i++) checksumFp += dotOnceVector(a, b, n);
    }

    private static void warmupDouble() {
        int n = 100_000;
        double[] a = new double[n];
        double[] b = new double[n];
        fill(a, b, n, 0x27182818L + n);
        for (int i = 0; i < 10; i++) checksumFp += dotOnceVector(a, b, n);
    }

    private static void benchByte(int n) {
        byte[] a = new byte[n];
        byte[] b = new byte[n];
        fill(a, b, n, 0xC0FFEE + n);

        int reps = repsForN(n);
        long sum = 0;
        long t0 = System.nanoTime();
        for (int r = 0; r < reps; r++) sum += dotOnce(a, b, n);
        long t1 = System.nanoTime();

        checksumInt += sum;
        long total = t1 - t0;
        double avg = (double) total / (double) reps;
        System.out.printf(Locale.ROOT, "JavaVector,signed_char,%d,%d,%d,%.2f,%d%n", n, reps, total, avg, checksumInt);
    }

    private static void benchShort(int n) {
        short[] a = new short[n];
        short[] b = new short[n];
        fill(a, b, n, 0xBADC0DE + n);

        int reps = repsForN(n);
        long sum = 0;
        long t0 = System.nanoTime();
        for (int r = 0; r < reps; r++) sum += dotOnce(a, b, n);
        long t1 = System.nanoTime();

        checksumInt += sum;
        long total = t1 - t0;
        double avg = (double) total / (double) reps;
        System.out.printf(Locale.ROOT, "JavaVector,short,%d,%d,%d,%.2f,%d%n", n, reps, total, avg, checksumInt);
    }

    private static void benchInt(int n) {
        int[] a = new int[n];
        int[] b = new int[n];
        fill(a, b, n, 0x12345678L + n);

        int reps = repsForN(n);
        long sum = 0;
        long t0 = System.nanoTime();
        for (int r = 0; r < reps; r++) sum += dotOnce(a, b, n);
        long t1 = System.nanoTime();

        checksumInt += sum;
        long total = t1 - t0;
        double avg = (double) total / (double) reps;
        System.out.printf(Locale.ROOT, "JavaVector,int,%d,%d,%d,%.2f,%d%n", n, reps, total, avg, checksumInt);
    }

    private static void benchFloat(int n) {
        float[] a = new float[n];
        float[] b = new float[n];
        fill(a, b, n, 0x31415926L + n);

        int reps = repsForN(n);
        double sum = 0.0;
        long t0 = System.nanoTime();
        for (int r = 0; r < reps; r++) sum += dotOnceVector(a, b, n);
        long t1 = System.nanoTime();

        checksumFp += sum;
        long total = t1 - t0;
        double avg = (double) total / (double) reps;
        System.out.printf(Locale.ROOT, "JavaVector,float,%d,%d,%d,%.2f,%.6f%n", n, reps, total, avg, checksumFp);
    }

    private static void benchDouble(int n) {
        double[] a = new double[n];
        double[] b = new double[n];
        fill(a, b, n, 0x27182818L + n);

        int reps = repsForN(n);
        double sum = 0.0;
        long t0 = System.nanoTime();
        for (int r = 0; r < reps; r++) sum += dotOnceVector(a, b, n);
        long t1 = System.nanoTime();

        checksumFp += sum;
        long total = t1 - t0;
        double avg = (double) total / (double) reps;
        System.out.printf(Locale.ROOT, "JavaVector,double,%d,%d,%d,%.2f,%.6f%n", n, reps, total, avg, checksumFp);
    }

    public static void main(String[] args) {
        printHeader();

        warmupByte();
        warmupShort();
        warmupInt();
        warmupFloat();
        warmupDouble();

        for (int n : SIZES) benchByte(n);
        for (int n : SIZES) benchShort(n);
        for (int n : SIZES) benchInt(n);
        for (int n : SIZES) benchFloat(n);
        for (int n : SIZES) benchDouble(n);

        System.err.printf(Locale.ROOT, "checksum_int=%d checksum_fp=%.6f%n", checksumInt, checksumFp);
    }
}
