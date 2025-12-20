import org.jetbrains.annotations.NotNull
import java.io.File
import java.nio.file.Files
import java.util.*
import java.util.stream.LongStream
import kotlin.concurrent.atomics.AtomicLong
import kotlin.concurrent.atomics.ExperimentalAtomicApi
import kotlin.concurrent.atomics.fetchAndIncrement
import kotlin.io.path.Path
import kotlin.math.roundToLong
import kotlin.math.sqrt
import kotlin.streams.toList

@OptIn(ExperimentalAtomicApi::class)
fun main() {
    val path = Path("./primes.txt")
    if (!Files.exists(path))
        Files.createFile(path)
    val knownPrimes = Files.readAllLines(path).stream().mapToLong { it.toLong() }.toList().toMutableList()
    val downerLimit: Long = 2
    var upperLimit: Long

    while (true) if (knownPrimes.size > 1) {
        clearConsole()
        print("Do you want to start all over again? (y/N): ")
        val response = System.console().readLine().trim { it <= ' ' }.lowercase(Locale.getDefault())[0]
        if (response == 'y' || response == 'n') {
            if (response == 'y') knownPrimes.clear()

            break
        }
    } else break
    while (true) {
        Main.clearConsole()
        print("Do you want ot set upper limit?(y/N): ")
        val response = System.console().readLine().trim { it <= ' ' }.lowercase(Locale.getDefault())[0]
        if (response == 'y' || response == 'n') {
            if (response == 'y') {
                print("Enter upper limit: ")
                upperLimit = System.console().readLine().trim { it <= ' ' }.lowercase(Locale.getDefault()).toLong()
            } else upperLimit = Long.MAX_VALUE
            break
        }
    }
    val startTime = System.nanoTime()

    val counter = AtomicLong(0);

    val newPrimes = listOf<Long>().toMutableList()
    LongStream.range(downerLimit, upperLimit).boxed().parallel().forEachOrdered {
        if (knownPrimes.contains(it)) {
            return@forEachOrdered
        }
        if (!it.isPrime()) {
            return@forEachOrdered
        }
        newPrimes.add(it)
        print("${counter.fetchAndIncrement()} Fount new: $it\n")
    }

    val endtime = System.nanoTime()
    val duration = ((endtime - startTime).toDouble()) / 1000000
    println("Time of execution: " + duration + "ms")
    newPrimes.parallelStream().forEach {
        if(it != null)
            knownPrimes.add(it) }
    try {
        val sortedList = knownPrimes.filterNotNull().sorted()
        File(path.toString()).writeText(
            sortedList.joinToString("\n")
        )
    }catch (e: NullPointerException){
        println("Error: $e")
    }



}

fun Long.isPrime(): Boolean {
    val maxPart = sqrt(this.toDouble()).roundToLong()

    for (i in 2..maxPart) {
        if (this % i == 0L) {
            return false
        }
    }
    return true
}

fun clearConsole() {
    print("\u001b[H\u001b[2J")
    System.out.flush()
}

