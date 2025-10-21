package com.example.ifc

import com.fasterxml.jackson.annotation.JsonProperty
import com.fasterxml.jackson.annotation.JsonValue
import com.fasterxml.jackson.module.kotlin.jacksonObjectMapper
import com.fasterxml.jackson.module.kotlin.registerKotlinModule
import io.ktor.server.application.*

private val mapper by lazy { jacksonObjectMapper().registerKotlinModule() }

data class ErrorResponse(
    val error: String,
    val message: String
)

data class ConversionOptions(
    @JsonProperty("units")
    val units: String = "meter",
    @JsonProperty("triangulation_tolerance")
    val triangulationTolerance: Double = 0.001,
    @JsonProperty("weld_vertices")
    val weldVertices: Boolean = true,
    @JsonProperty("include_properties")
    val includeProperties: Boolean = false,
    @JsonProperty("lod")
    val lod: LevelOfDetail = LevelOfDetail.MEDIUM
) {
    enum class LevelOfDetail(@JsonValue val value: String) {
        LOW("low"),
        MEDIUM("medium"),
        HIGH("high")
    }

    fun toJson(): String = mapper.writeValueAsString(this)

    companion object {
        fun fromCall(call: ApplicationCall): ConversionOptions {
            val parameters = call.parameters
            val lod = parameters["lod"]?.uppercase()?.let {
                runCatching { LevelOfDetail.valueOf(it) }.getOrNull()
            } ?: LevelOfDetail.MEDIUM

            return ConversionOptions(
                units = parameters["units"] ?: "meter",
                triangulationTolerance = parameters["triangulation_tolerance"]?.toDoubleOrNull() ?: 0.001,
                weldVertices = parameters["weld_vertices"]?.toBooleanStrictOrNull() ?: true,
                includeProperties = parameters["include_properties"]?.toBooleanStrictOrNull() ?: false,
                lod = lod
            )
        }
    }
}

private fun String.toBooleanStrictOrNull(): Boolean? = when (lowercase()) {
    "true", "1", "yes", "y" -> true
    "false", "0", "no", "n" -> false
    else -> null
}
