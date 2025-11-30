package com.example.serialportscannerjunsun

import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import kotlinx.coroutines.*
import java.io.File

class MainActivity : AppCompatActivity() {

    private lateinit var btnScan: Button
    private lateinit var tvLog: TextView

    private val commonBauds = intArrayOf(9600, 38400, 115200)
    private val commonPorts = arrayOf(
        "/dev/ttyS0",
        "/dev/ttyS1",
        "/dev/ttyS2",
        "/dev/ttyS3",
        "/dev/ttyMT0",
        "/dev/ttyMT1",
        "/dev/ttyUSB0"
    )

    private val scope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        btnScan = findViewById(R.id.btnScan)
        tvLog = findViewById(R.id.tvLog)

        btnScan.setOnClickListener {
            tvLog.text = "Starting scan...\n"
            scanPortsAuto()
        }
    }

    private fun log(s: String) {
        runOnUiThread { tvLog.append(s + "\n") }
    }

    private fun scanPortsAuto() {
        scope.launch {
            // scan listed ports first
            for (p in commonPorts) {
                for (baud in commonBauds) {
                    tryPort(p, baud)
                }
            }

            // scan /dev for tty* if not found
            val dev = File("/dev")
            dev.listFiles()?.forEach { f ->
                val name = f.name
                if (name.startsWith("tty")) {
                    for (baud in commonBauds) tryPort("/dev/$name", baud)
                }
            }

            log("Scan complete")
        }
    }

    private fun tryPort(path: String, baud: Int) {
        try {
            if (!File(path).exists()) return
            log("Trying $path @ $baud")
            val sp = SerialPort.open(path, baud, 0)
            if (sp != null) {
                log("OPENED $path @ $baud")
                // read a bit non-blocking
                val input = sp.inputStream
                val buffer = ByteArray(256)
                val len = input.read(buffer)
                if (len > 0) {
                    val bytes = buffer.copyOf(len)
                    val hex = bytes.joinToString(" ") { String.format("%02X", it) }
                    log("RX ($path): $hex")
                } else log("No data yet on $path")
                sp.close()
            }
        } catch (e: Exception) {
            log("Err $path@$baud: ${e.message}")
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        scope.cancel()
    }
}
