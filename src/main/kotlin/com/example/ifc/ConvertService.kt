package com.example.ifc

import java.nio.file.Path

class ConvertService {
    fun runConversion(inputPath: Path, outputPath: Path, optionsJson: String): Int {
        return IfcOpenShellBridge.convertIfcToGlb(
            inputPath.toAbsolutePath().toString(),
            outputPath.toAbsolutePath().toString(),
            optionsJson
        )
    }
}

