using System.Threading;
using SwiftExcel;


namespace primes_generator
{
    class Core
    {   
        static readonly List<long> primes = new() { 2};
        static bool  areWeSaving = false;
        static int perc;
        static long howMany;

        public static void Main()
        {
            StartMenu();



            long pc = primes.Count;

            for (long i = 3; ; i++)
            {
                if (pc >= howMany)
                    break;



                if (IsPrime(i))
                {
                    primes.Add(i); 
                    pc++;

                    if (pc % perc == 0 || pc == howMany)
                    {
                        Console.WriteLine($"{pc}\t{(int)((double)pc / (double)howMany * 100)}%");
                    }
                }
            }

            if (areWeSaving)
                Save();
        }

        static bool IsDivisible(long num, long prime)
        {
            if (num % prime == 0)
            {
                return true;
            }
            return false;
        }

        static void Save()
        {
            List<string> primesToSave = new();
            for (int index = 0; index < primes.Count; index++)
            {
                long p = primes[index];
                string s = p.ToString();
                primesToSave.Add(s);
                if (index % perc == 0 || index == howMany - 1)
                    Console.WriteLine($"{p}\t{(int)(((double)index / (double)(howMany-1)) * 100)}%");
            }
            File.WriteAllLines("PrimeNumbersOut.txt", primesToSave);
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

            Console.Write("What percent should data for statistics be collected ");

            try
            {
                perc = (int)((double)howMany / (1 / (Convert.ToDouble(Console.ReadLine()) / 100)));
                if (perc == howMany)
                    perc = 1;
            }
            catch
            {
                Console.WriteLine("Wrong value!!!");
                Console.ReadKey();
                return;
            }
            if (perc <= 0)
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

        static bool IsPrime(long i)
        {
            foreach (var j in primes)
            {
                if (IsDivisible(i, j))
                {
                    return false;
                }
            }
            return true;
        }
    }
}



