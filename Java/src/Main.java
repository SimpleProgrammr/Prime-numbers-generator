import java.io.IO;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;
import java.util.concurrent.atomic.AtomicLong;
import java.util.stream.Collectors;
import java.util.stream.LongStream;

public class Main {
    void main(String[] args) throws IOException {
        boolean toClear = true;
        var path = Path.of("./knownNumbers.txt");
        long upperLimit = 0, lesserLimit = Long.MAX_VALUE;


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
                                path = Path.of(args[++i]);
                            break;

                    }

                    break;
            }
        }


        if (!Files.exists(path)) {
            Files.createFile(path);
        }
        var alreadyKnownNumbers = new ArrayList<Long>();
        Files.readAllLines(path).stream().mapToLong(Long::parseLong).forEach(alreadyKnownNumbers::add);
        if (toClear)
            alreadyKnownNumbers.clear();




        List<Long> newFound = new ArrayList<>();
        AtomicLong counter = new AtomicLong();
        counter.set(0);

        Timer timer = new Timer();

        long startTime = System.nanoTime();
        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                System.out.println("Already found " + counter + " Prime numbers\t\t|\t" + ((double)(System.nanoTime() - startTime)/1000000) + "ms");
            }
        };
        timer.scheduleAtFixedRate(task, 250, 250);


        LongStream.range(lesserLimit, upperLimit).parallel().forEach(num -> {
            if (alreadyKnownNumbers.contains(num))
                return;
            if (!isPrime(num))
                return;
            newFound.add(num);
            counter.incrementAndGet();
        });
        long endtime = System.nanoTime();


        timer.cancel();
        double duration = ((double) (endtime - startTime)) / 1000000;
        IO.println("Time of execution: " + duration + "ms");
        IO.println("Found "+ newFound.size()+" prime numbers");

        newFound.addAll(alreadyKnownNumbers);
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
}