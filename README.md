# Cthulhu

Grammar is defined in ctu.g4 but the parser is not generated using antlr due to its ludicrous runtime

```sh
kotlinc Main.kt -include-runtime -d ctc.jar && java -jar -ea ctc.jar ../tests/Main.ct```