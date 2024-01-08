plugins {
    id("java")
    id("application")
}

application {
    mainClass = "SampleLauncher"
}

dependencies {
    implementation(project(":api"))
}
