import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.*;
import java.util.concurrent.atomic.AtomicLong;
import java.util.stream.Collectors;
import java.util.stream.LongStream;

public class Main {
    void main(String[] args) throws IOException {
        boolean toClear = true;
        final Path path;
        long upperLimit = 0, lesserLimit = Long.MAX_VALUE;

        var temp = Path.of("./knownNumbers.txt");
        for (int i = 0; i < args.length; i++) {
            switch (i) {
                case 0:
                    lesserLimit = Math.max(2, Long.parseLong(args[0]));
                    break;
                case 1:
                    upperLimit = Math.min(Long.MAX_VALUE, Long.parseLong(args[1]));
                    break;
                default:

                    switch (args[i]) {
                        case "--continue":
                        case "-c":
                            toClear = false;
                            break;

                        case "--sourceFile":
                        case "-sF":
                            if (toClear)
                                break;
                            if (i + 1 >= args.length)
                                break;
                            if (Files.exists(Path.of(args[i + 1])))
                                temp = Path.of(args[++i]);
                            break;

                    }

                    break;
            }
        }
        path = temp;


        if (!Files.exists(path)) {
            Files.createFile(path);
        }
        Files.writeString(path, "");
        var alreadyKnownNumbers = new ArrayList<Long>();
        Files.readAllLines(path).stream().mapToLong(Long::parseLong).forEach(alreadyKnownNumbers::add);
        if (toClear)
            alreadyKnownNumbers.clear();




        List<Long> newFound = new ArrayList<>();
        AtomicLong counter = new AtomicLong();
        final long predictedPrimesAmount = (long) (upperLimit/Math.log(upperLimit) - lesserLimit/Math.log(lesserLimit));  //π(b) - π(a) ≈ b/ln(b) - a/ln(a)

        counter.set(0);

        Timer timer = new Timer();
        Timer saver = new Timer();

        long startTime = System.nanoTime();
        TimerTask postStatisticsTask = new TimerTask() {
            @Override
            public void run() {
                long c = counter.longValue();
                clearScreen();
                System.out.print("Already found " + c + " / "+ predictedPrimesAmount +" PRime numbers ( "+ ((double)c/predictedPrimesAmount)*100 +"% )\t\t|\t" + ((double)(System.nanoTime() - startTime)/1000000) + "ms\t\t\t");
            }
        };
        TimerTask saveToFileTask = new TimerTask() {
            @Override
            public void run() {
                synchronized (newFound) {
                    try {
                        amendListToFile(path,newFound);

                    } catch (IOException e) {
                        throw new RuntimeException(e);
                    }
                }
            }
        } ;
        timer.scheduleAtFixedRate(postStatisticsTask, 0, 500);
        saver.scheduleAtFixedRate(saveToFileTask, 30*1000, 30*1000);


        LongStream.range(lesserLimit, upperLimit).parallel().forEach(num -> {
            if (alreadyKnownNumbers.contains(num))
                return;
            if (!isPrime(num))
                return;
            synchronized (newFound) {
                newFound.add(num);
                counter.incrementAndGet();
            }
        });
        long endtime = System.nanoTime();

        timer.cancel();
        saver.cancel();
        amendListToFile(path,newFound);

        double duration = ((double) (endtime - startTime)) / 1000000;
        System.out.println("\n\nTime of execution: " + duration + "ms");
        System.out.println("\n\nFound "+ counter +" prime numbers");
        newFound.clear();
        newFound.addAll(Files.readAllLines(path).stream().map(Long::parseLong).toList());
        newFound.addAll(alreadyKnownNumbers.stream().toList());
        newFound.removeIf(Objects::isNull);

        if(!newFound.stream().filter(Objects::isNull).toList().isEmpty()){
            System.out.println("ERROR: Null element");
            return;
        }


        Collections.sort(newFound);
        Files.write(path, newFound.stream().map(Object::toString).collect(Collectors.toList()));

    }


    public static boolean isPrime(long num) {
        long maxPart = Math.round(Math.sqrt(num));

        for (long i = 2; i <= maxPart; i++) {
            if (num % i == 0) {
                return false;
            }
        }
        return true;
    }

    public static void amendListToFile(Path path, List<Long> List) throws IOException {
        Files.write(path, List.stream().map(Object::toString).collect(Collectors.toList()), StandardOpenOption.APPEND);
        System.out.println("\rSaved "+List.size()+" Elements");
        List.clear();
    }

    public static void clearScreen() {
        try {
            // Dla Windows
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                new ProcessBuilder("cmd", "/c", "cls").inheritIO().start().waitFor();
            } else {
                // Dla Unix/Linux/Mac
                new ProcessBuilder("clear").inheritIO().start().waitFor();
            }
        } catch (Exception e) {
            // Jeśli nie można uruchomić komendy, użyj ANSI
            System.out.print("\033[H\033[2J");
            System.out.flush();
        }
    }
}