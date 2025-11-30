package com.example.serialportscannerjunsun

import java.io.FileDescriptor
import java.io.FileInputStream
import java.io.FileOutputStream

/**
 * Minimal wrapper around native serial port helper (libserial_port).
 * This follows the pattern of the common android-serialport-api.
 */
class SerialPort private constructor(private val fd: FileDescriptor, private val input: FileInputStream, private val output: FileOutputStream) {

    companion object {
        init {
            System.loadLibrary("serial_port")
        }

        @JvmStatic
        external fun open(path: String, baudrate: Int, flags: Int): SerialPort?

        @JvmStatic
        external fun close(fd: FileDescriptor)
    }

    fun getFileDescriptor(): FileDescriptor = fd
    val inputStream get() = input
    val outputStream get() = output

    fun close() {
        try { input.close() } catch (_: Exception) {}
        try { output.close() } catch (_: Exception) {}
        try { close(fd) } catch (_: Exception) {}
    }
}
