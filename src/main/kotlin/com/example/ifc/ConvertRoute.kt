package com.example.ifc

import io.ktor.http.*
import io.ktor.http.content.*
import io.ktor.server.application.*
import io.ktor.server.request.*
import io.ktor.server.response.*
import io.ktor.server.routing.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.nio.file.Files
import java.nio.file.Path
import java.time.Duration
import java.time.Instant
import java.util.UUID

fun Route.convertRoute(service: ConvertService) {
    post("/convert") {
        val dataDir = Path.of(System.getenv("DATA_DIR") ?: "/data")
        val inDir = Path.of(System.getenv("IN_DIR") ?: dataDir.resolve("in").toString())
        val outDir = Path.of(System.getenv("OUT_DIR") ?: dataDir.resolve("out").toString())

        Files.createDirectories(inDir)
        Files.createDirectories(outDir)

        val uuid = UUID.randomUUID().toString()
        val inputPath = inDir.resolve("$uuid.ifc")
        val outputPath = outDir.resolve("$uuid.glb")

        val maxUploadMb = System.getenv("MAX_UPLOAD_MB")?.toLongOrNull() ?: 1024L
        val maxUploadBytes = maxUploadMb * 1024 * 1024

        var originalFileName: String? = null
        var receivedFile = false
        var payloadTooLarge = false

        val multipart = call.receiveMultipart()
        multipart.forEachPart { part ->
            when (part) {
                is PartData.FileItem -> {
                    if (part.name == "file" && !receivedFile) {
                        originalFileName = part.originalFileName
                        try {
                            writePartToFile(part, inputPath, maxUploadBytes)
                            receivedFile = true
                        } catch (ex: PayloadTooLargeException) {
                            payloadTooLarge = true
                        }
                    }
                }
                else -> Unit
            }
            part.dispose()
        }

        if (payloadTooLarge) {
            Files.deleteIfExists(inputPath)
            return@post call.respond(HttpStatusCode.PayloadTooLarge, ErrorResponse("payload_too_large", "Upload exceeds limit of $maxUploadMb MB"))
        }

        if (!receivedFile) {
            Files.deleteIfExists(inputPath)
            return@post call.respond(HttpStatusCode.BadRequest, ErrorResponse("missing_file", "No IFC file was provided in the 'file' field."))
        }

        val fileName = (originalFileName ?: "upload.ifc").lowercase()
        if (!fileName.endsWith(".ifc")) {
            Files.deleteIfExists(inputPath)
            return@post call.respond(HttpStatusCode.UnsupportedMediaType, ErrorResponse("invalid_type", "Only .ifc files are accepted."))
        }

        val options = ConversionOptions.fromCall(call)
        val optionsJson = options.toJson()
        val start = Instant.now()
        val resultCode = withContext(Dispatchers.IO) {
            service.runConversion(inputPath, outputPath, optionsJson)
        }
        val duration = Duration.between(start, Instant.now())

        when (resultCode) {
            0 -> {
                val sizeBytes = Files.size(outputPath)
                call.respond(
                    mapOf(
                        "status" to "ok",
                        "inputFile" to (originalFileName ?: fileName),
                        "outputFile" to outputPath.toString(),
                        "durationMs" to duration.toMillis(),
                        "sizeBytes" to sizeBytes
                    )
                )
            }
            1 -> call.respond(HttpStatusCode.BadRequest, ErrorResponse("input_not_found", "Input IFC file could not be found by the native converter."))
            5 -> call.respond(HttpStatusCode.BadRequest, ErrorResponse("invalid_options", "Conversion options were invalid."))
            2, 3, 4 -> call.respond(HttpStatusCode.InternalServerError, ErrorResponse("conversion_failed", "Native converter failed with code $resultCode."))
            else -> call.respond(HttpStatusCode.InternalServerError, ErrorResponse("unknown_error", "Native converter returned unexpected code $resultCode."))
        }
    }
}

private suspend fun writePartToFile(part: PartData.FileItem, target: Path, maxBytes: Long) {
    withContext(Dispatchers.IO) {
        Files.newOutputStream(target).use { output ->
            part.streamProvider().use { input ->
                val buffer = ByteArray(DEFAULT_BUFFER_SIZE)
                var total = 0L
                while (true) {
                    val read = input.read(buffer)
                    if (read == -1) break
                    total += read
                    if (total > maxBytes) {
                        throw PayloadTooLargeException()
                    }
                    output.write(buffer, 0, read)
                }
            }
        }
    }
}

private class PayloadTooLargeException : RuntimeException()
