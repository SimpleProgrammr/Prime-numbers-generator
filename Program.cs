
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

class PrimeGenerator
{
    static long howMany = 1;
    static bool areWeSaving = true;
    static readonly long numberOfThreads = Environment.ProcessorCount;
    static List<long> primeList = new();
    static short progress = 0;

    static void Main()
    { 

        StartMenu();

        ConcurrentBag<long> primes = new();
        CancellationTokenSource cancellationTokenSource = new();

        Console.WriteLine($"Generowanie liczb pierwszych w zakresie od 2 do {howMany}...");

        List<Task> tasks = new();

        for (int i = 0; i < numberOfThreads; i++)
        {
            int start = 2 + i; // Rozpoczęcie od różnych reszt z dzielenia
            Task task = Task.Factory.StartNew(() => FindPrimesInRange(start, howMany, primes, cancellationTokenSource.Token));
            tasks.Add(task);
        }

        Task.WaitAll(tasks.ToArray());
        cancellationTokenSource.Cancel(); // Anulowanie pozostałych zadań

        primeList = primes.OrderBy(p => p).ToList();

        Console.WriteLine();
        if ( areWeSaving ) 
            Save();
    }

    static void FindPrimesInRange(long start, long end, ConcurrentBag<long> primes, CancellationToken cancellationToken)
    {
        for (long i = start; i <= end; i += Environment.ProcessorCount)
        {
            if (IsPrime(i))
            {
                primes.Add(i);
            }
            if (i % (long)((double)howMany * 0.01) == 0)
            {
                progress++;
                Console.WriteLine(progress+"%");
            }
            if (cancellationToken.IsCancellationRequested)
            {
                return; // Przerwanie, jeśli żądanie anulowania zostało wysłane
            }
        }
    }

    static bool IsPrime(long number)
    {
        if (number <= 1)
        {
            return false;
        }

        if (number <= 3)
        {
            return true;
        }

        if (number % 2 == 0 || number % 3 == 0)
        {
            return false;
        }

        int i = 5;
        while (i * i <= number)
        {
            if (number % i == 0 || number % (i + 2) == 0)
            {
                return false;
            }
            i += 6;
        }

        return true;
    }
    static void StartMenu()
    {
        Console.Write("Write a number of primes to generate(min. 1)");
        try
        {
            howMany = Convert.ToInt64(Console.ReadLine());

        }
        catch
        {
            Console.WriteLine("Wrong value!!!");
            Console.ReadKey();
            return;
        }
        if (howMany <= 0)
        {
            Console.WriteLine("Wrong value!!!");
            Console.ReadKey();
            return;
        }

        Console.Write("Do you want to save resoult to .txt file?(Y/n) ");
        if (Console.ReadKey().Key == ConsoleKey.Y)
            areWeSaving = true;
        Console.WriteLine();

    }

    static void Save()
    {
        List<string> primesToSave = new();

        foreach (var prime in primeList)
            primesToSave.Add(prime.ToString());

        File.WriteAllLines("C:\\Users\\micha\\Desktop\\Primes.txt", primesToSave);
        Console.WriteLine("\nSaved\n");
    }
}





