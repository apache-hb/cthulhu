# Cthulhu

Grammar is defined in ctu.g4 but the parser is not generated using antlr due to its ludicrous runtime

```sh
kotlinc Main.kt -d ctc.jar && kotlin -classpath ctc.jar MainKt ../tests/Main.ct```