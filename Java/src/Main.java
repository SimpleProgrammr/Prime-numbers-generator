import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;
import java.util.concurrent.atomic.AtomicLong;
import java.util.stream.Collectors;
import java.util.stream.LongStream;

public class Main {
    public static void main() throws IOException {
        var path = Path.of("./knownNumbers.txt");
        if(!Files.exists(path))
        {
            Files.createFile(path);
        }
        var alreadyKnownNumbers = Files.readAllLines(path);
        long upperLimit;
        while (true)
            if (alreadyKnownNumbers.size() > 1) {
                clearConsole();
                System.out.print("Do you want to start all over again? (y/N): ");
                var response = System.console().readLine().trim().toLowerCase().charAt(0);
                if (response == 'y' || response == 'n') {
                    if (response == 'y')
                        alreadyKnownNumbers.clear();

                    break;
                }

            }else
                break;
        while (true) {
            clearConsole();
            System.out.print("Do you want ot set upper limit?(y/N): ");
            var response = System.console().readLine().trim().toLowerCase().charAt(0);
            if (response == 'y' || response == 'n') {
                if (response == 'y') {
                    System.out.print("Enter upper limit: ");
                    upperLimit = Long.parseLong(System.console().readLine().trim().toLowerCase());
                }else
                    upperLimit = Long.MAX_VALUE;
                break;

            }

        }
        long startTime = System.nanoTime();

        List<Long> newFound =  new ArrayList<>();
        AtomicLong counter = new AtomicLong();
        counter.set(0);
        LongStream.range(2, upperLimit).boxed().parallel().forEachOrdered(num ->{
                if(alreadyKnownNumbers.contains(num.toString()))
                    return;
                if(!isPrime(num))
                    return;
                newFound.add(num);
                long n = counter.getAndIncrement();
                System.out.println(n + ". Found new prime: "+num);
        });
        long endtime = System.nanoTime();
        double duration = ((double)(endtime - startTime))/1000;
        System.out.println("Time of execution: " + duration + "ms");

        alreadyKnownNumbers.forEach(num -> newFound.add(Long.parseLong( num)));
        Collections.sort(newFound);
        Files.write(path,newFound.stream().map(Object::toString).collect(Collectors.toList()));

    }

    public static void clearConsole() {
        System.out.print("\033[H\033[2J");
        System.out.flush();
    }



    public static boolean isPrime(long num){
        long maxPart= Math.round(Math.sqrt(num));

        for (long i = 2; i <= maxPart; i++){
            if(num%i ==0){
                return false;
            }
        }
        return true;
    }


}