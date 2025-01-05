package random.analyzer;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class App {

    public static void main(String[] args) {
        final List<String> inputFiles = new ArrayList<>();

        inputFiles.addAll(Arrays.asList(args)); // Si pas de switch pour présence d'options, pour tout mettre (args donnés au main)
        //inputFiles.add(".\\ressources\\corpus1.txt");
        inputFiles.add(".\\ressources\\corpus2.txt");

        System.out.println("Lancement de l'analyseur de corpus.");
        new Analyzer(inputFiles);   // Run : Input -> ... -> Output
        System.out.println("Au revoir !");
    }
}
