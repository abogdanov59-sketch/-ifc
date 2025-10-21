package com.example.ifc

import com.fasterxml.jackson.databind.SerializationFeature
import com.fasterxml.jackson.module.kotlin.registerKotlinModule
import io.ktor.serialization.jackson.*
import io.ktor.server.application.*
import io.ktor.server.engine.embeddedServer
import io.ktor.server.netty.Netty
import io.ktor.server.plugins.callloging.*
import io.ktor.server.plugins.compression.*
import io.ktor.server.plugins.contentnegotiation.*
import io.ktor.server.plugins.cors.routing.*
import io.ktor.server.plugins.statuspages.*
import io.ktor.server.response.*
import io.ktor.server.routing.*
import org.slf4j.LoggerFactory
import org.slf4j.event.Level

fun main() {
    val port = System.getenv("PORT")?.toIntOrNull() ?: 8080
    val logger = LoggerFactory.getLogger("ifc-glb-service")
    runCatching { IfcOpenShellBridge.ensureLoaded() }
        .onFailure { logger.warn("Failed to load native library: ${it.message}") }

    embeddedServer(Netty, port = port, host = "0.0.0.0") {
        configureApplication()
    }.start(wait = true)
}

fun Application.configureApplication() {
    install(CallLogging) {
        level = when (System.getenv("LOG_LEVEL")?.uppercase()) {
            "TRACE" -> Level.TRACE
            "DEBUG" -> Level.DEBUG
            "WARN" -> Level.WARN
            "ERROR" -> Level.ERROR
            else -> Level.INFO
        }
        mdc("requestId") { call -> call.request.headers["X-Request-ID"] }
    }

    install(ContentNegotiation) {
        jackson {
            registerKotlinModule()
            configure(SerializationFeature.INDENT_OUTPUT, true)
        }
    }

    install(CORS) {
        anyHost()
        allowHeader("*")
        allowCredentials = false
    }

    install(StatusPages) {
        exception<Throwable> { call, cause ->
            call.application.environment.log.error("Unhandled exception", cause)
            call.respond(
                status = io.ktor.http.HttpStatusCode.InternalServerError,
                message = ErrorResponse("internal_error", cause.message ?: "Unexpected error")
            )
        }
    }

    install(Compression) {
        gzip()
    }

    routing {
        get("/health") {
            call.respond(mapOf("status" to "ok"))
        }
        convertRoute(ConvertService())
    }
}
