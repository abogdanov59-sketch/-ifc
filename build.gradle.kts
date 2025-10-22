import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    kotlin("jvm") version "1.9.22"
    id("application")
    id("com.github.johnrengelman.shadow") version "8.1.1"
}

repositories {
    mavenCentral()
}

application {
    mainClass.set("com.example.ifc.ApplicationKt")
}

java {
    toolchain {
        languageVersion.set(JavaLanguageVersion.of(17))
    }
}

val ktorVersion = "2.3.7"

dependencies {
    implementation("io.ktor:ktor-server-core-jvm:$ktorVersion")
    implementation("io.ktor:ktor-server-netty-jvm:$ktorVersion")
    implementation("io.ktor:ktor-server-call-logging-jvm:$ktorVersion")
    implementation("io.ktor:ktor-server-content-negotiation-jvm:$ktorVersion")
    implementation("io.ktor:ktor-serialization-jackson-jvm:$ktorVersion")
    implementation("io.ktor:ktor-server-status-pages-jvm:$ktorVersion")
    implementation("io.ktor:ktor-server-cors-jvm:$ktorVersion")
    implementation("io.ktor:ktor-server-compression-jvm:$ktorVersion")
    implementation("ch.qos.logback:logback-classic:1.4.14")
    implementation("com.fasterxml.jackson.module:jackson-module-kotlin:2.15.3")

    testImplementation(kotlin("test"))
    testImplementation("io.ktor:ktor-server-tests-jvm:$ktorVersion")
    testImplementation("io.ktor:ktor-client-content-negotiation:$ktorVersion")
    testImplementation("io.ktor:ktor-client-java:$ktorVersion")
}

sourceSets {
    main {
        java {
            setSrcDirs(listOf("src/main/java"))
        }
        kotlin {
            setSrcDirs(listOf("src/main/kotlin"))
        }
        resources {
            srcDir("src/main/resources")
        }
    }
}

tasks.withType<KotlinCompile> {
    kotlinOptions.jvmTarget = "17"
}

tasks.register<JavaCompile>("jniHeaders") {
    description = "Generate JNI headers for the IfcOpenShell bridge"
    group = "build"
    source = fileTree("src/main/java")
    classpath = files()
    destinationDir = file("native/include")
    options.compilerArgs = listOf("-h", destinationDir.absolutePath)
}

tasks.register<Exec>("buildNative") {
    description = "Configure the JNI native library via CMake"
    group = "build"
    dependsOn("jniHeaders")
    workingDir = file("native")
    commandLine = listOf("cmake", "-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=Release")
}

tasks.register<Exec>("compileNative") {
    description = "Compile the JNI native library"
    group = "build"
    dependsOn("buildNative")
    workingDir = file("native")
    commandLine = listOf("cmake", "--build", "build", "--config", "Release")
}

tasks.named("shadowJar") {
    dependsOn("compileNative")
}

