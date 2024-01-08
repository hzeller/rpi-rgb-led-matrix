import org.gradle.jvm.tasks.Jar

plugins {
    id("java")
}

val jniHeaderDir = layout.buildDirectory.dir("generated-jni-headers")
val compileJavaTasks = tasks.withType(JavaCompile::class.java) {
    outputs.dir(jniHeaderDir)
    options.compilerArgs.add("-h")
    options.compilerArgs.add(jniHeaderDir.get().asFile.path)
}

val CXX = "aarch64-linux-gnu-g++"

val compileNativeTask = tasks.register("compileNative") {
    // todo: jniHeaderDir should really be an output property of the compileJava task
    //  so this manual dependsOn is not needed anymore
    dependsOn(compileJavaTasks)
    inputs.dir(jniHeaderDir)

    val objectOutputDir = layout.buildDirectory.dir("cpp")

    doFirst {
        objectOutputDir.get().asFile.parentFile.mkdirs()
    }

    val cppFiles = fileTree(layout.projectDirectory.dir("src/main/cpp"))
    cppFiles.include("*.cpp")
    for (cppFile in cppFiles) {
        val objectFile = objectOutputDir.get()
            .file(cppFile.name.removeSuffix(".cpp") + ".o")
        inputs.file(cppFile)
        outputs.file(objectFile)
        doLast {
            exec {
                commandLine(
                    CXX,
                    "-c",
                    "-fPIC",
                    "-I${System.getProperty("java.home")}/include",
                    "-I${rootDir.parentFile.parent}/include",
                    "-I${jniHeaderDir.get().asFile.path}",
                    "-I${System.getProperty("java.home")}/include/linux",
                    cppFile,
                    "-o",
                    objectFile.asFile.path,
                )
            }
        }
    }
}

val linkerSearchPath = "${rootDir.parentFile.parent}/lib"

val linkNativeTask = tasks.register("linkNative", Exec::class.java) {
    val objectFiles = files(compileNativeTask.map { it.outputs })
    objectFiles.filter { it.name.endsWith(".o") }

    val nativeLibOutput = layout.buildDirectory
        .file("jnilib/lib${project.name.trim('-', '_').lowercase()}.so")

    inputs.files(objectFiles)
    outputs.file(nativeLibOutput)

    commandLine(
        CXX,
        "-shared",
        "-fPIC",
        "-L$linkerSearchPath",
        "-o",
    )
    argumentProviders.add(CommandLineArgumentProvider {
        buildList {
            add(nativeLibOutput.get().asFile.path)
            addAll(objectFiles.map { it.path })
            addAll(listOf("-lc", "-l", "rgbmatrix", "-lrt", "-lm", "-lpthread"))
        }
    })
}

tasks.getByName("jar", Jar::class) {
    into("resources/") {
        from(linkNativeTask.map { it.outputs.files })
        from(linkerSearchPath) {
            include("*.so.*")
        }
    }
}
