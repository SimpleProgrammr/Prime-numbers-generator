using System.Diagnostics;


namespace primes_generator
{
    class Core
    {
        static readonly List<long> primes = new() { 2 };
        static bool areWeSaving = false;
        static int perc;
        static long howMany;
        static bool add = true;
        static readonly List<Task> tasks = new() ;
        static long i;

        public static void Main(string[] args)
        {
            if(args.Length != 3)    
                StartMenu();
            else
            {
                howMany = int.Parse(args[0]);
                perc = (int)((double)howMany / (1 / (Convert.ToDouble(args[1]) / 100)));
                if (args[2] == "y" || args[2] == "Y")
                    areWeSaving = true;
                else
                    areWeSaving = false;
            }

            tasks.Add(IsDivisible(2));
            var clk = new Stopwatch();

            long pc = primes.Count;
            
            
            for (i = 3; ; i++)
            {
                clk.Start();

                if (pc >= howMany)
                    break;



                if (IsPrimeAsync().Result)
                {
                    primes.Add(i);
                    pc++;
                    tasks.Add(IsDivisible(i));
                    if (pc % perc == 0 || pc == howMany)
                    {
                        clk.Stop();
                        Console.WriteLine($"{pc}\t{(int)((double)pc / (double)howMany * 100)}%\t{clk.ElapsedMilliseconds}ms");

                    }
                }
            }

            if (areWeSaving)
                Save();

        }

        static Task IsDivisible( long prime)
        {
            if (i % prime == 0)
            {
                add = false;
                return Task.CompletedTask;
            }

            return Task.CompletedTask; ;
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
                    Console.WriteLine($"{p}\t{(int)(((double)index / (double)(howMany - 1)) * 100)}%");
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

        static async Task<bool> IsPrimeAsync()
        {
            Task.WaitAll(tasks.ToArray());

            if (add)
            {
                add = true;
                return true;

            }

            add = true;
            return false;
        }
    }
}

/*WOLNIEJSZE
static async Task<bool> IsPrimeAsync(long i)
        {

            List<Task> tasks = new();
            for (var j = 0; j < primes.Count; j++)
            {
                tasks.Add(IsDivisible(i, primes[j]));
            }
            await Task.WhenAll(tasks.ToArray());

            if (add)
            {
                add = true;
                return true;

            }

            add = true;
            return false;
        }
 */

/*Szybsze
        {

            foreach (var j in primes)
            {
                IsDivisible(i, j);
            }

            if (add)
            {
                add = true;
                return true;
            }

            add = true;
            return false;
        }
 */



