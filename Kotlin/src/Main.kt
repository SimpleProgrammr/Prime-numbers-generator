import java.io.File
import java.nio.file.Files
import java.nio.file.Path
import java.util.*
import java.util.stream.LongStream
import kotlin.concurrent.atomics.AtomicLong
import kotlin.concurrent.atomics.ExperimentalAtomicApi
import kotlin.concurrent.atomics.fetchAndIncrement
import kotlin.math.max
import kotlin.math.min
import kotlin.math.roundToLong
import kotlin.math.sqrt
import kotlin.streams.toList

@OptIn(ExperimentalAtomicApi::class)
fun main(args: Array<String>) {
    var toClear = true
    var path = Path.of("./knownNumbers.txt")
    var upperLimit: Long = 0
    var downerLimit = Long.MAX_VALUE

    var i = 0
    while (i < args.size) {
        when (i) {
            0 -> downerLimit = max(2, args[0].toLong())
            1 -> upperLimit = min(Long.MAX_VALUE, args[1].toLong())
            else -> when (args[i]) {
                "--continue", "-c" -> toClear = false
                "--sourceFile", "-sF" -> {
                    if (toClear) break
                    if (i + 1 >= args.size) break
                    if (Files.exists(Path.of(args[i + 1]))) path = Path.of(args[++i])
                }
            }

        }
        i++
    }



    if (!Files.exists(path)) {
        Files.createFile(path)
    }
    val knownPrimes = Files.readAllLines(path).stream().mapToLong { it.toLong() }.toList().toMutableList()
    if (toClear)
        knownPrimes.clear()



    val counter = AtomicLong(0)

    val newPrimes = listOf<Long>().toMutableList()

    val timer = Timer()

    val startTime = System.nanoTime()
    val task: TimerTask = object : TimerTask() {
        override fun run() {
            println("Already found " + counter + " Prime numbers\t\t|\t" + ((System.nanoTime() - startTime).toDouble() / 1000000) + "ms")
        }
    }
    timer.scheduleAtFixedRate(task, 250, 250)

    LongStream.range(downerLimit, upperLimit).parallel().forEach {
        if (knownPrimes.contains(it)) {
            return@forEach
        }
        if (!it.isPrime()) {
            return@forEach
        }
        newPrimes.add(it)
        counter.fetchAndIncrement()
        //print("${counter.fetchAndIncrement()} Fount new: $it\n")
    }
    val endtime = System.nanoTime()


    timer.cancel()
    val duration = ((endtime - startTime).toDouble()) / 1000000
    println("Time of execution: " + duration + "ms")
    println("Found " + newPrimes.size + " prime numbers")

    knownPrimes.addAll(newPrimes)
    try {
        val sortedList = knownPrimes.toList().sorted()
        File(path.toString()).writeText(
            sortedList.joinToString("\n")
        )
    } catch (e: NullPointerException) {
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

