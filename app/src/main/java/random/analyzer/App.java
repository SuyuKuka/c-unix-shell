package random.analyzer;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class App {

    public static void main(String[] args) {
        final List<String> inputFiles = new ArrayList<>();

        inputFiles.addAll(Arrays.asList(args)); // Si pas de switch pour présence d'options, pour tout mettre (args donnés au main)
        //inputFiles.add(".\\ressources\\poésie.txt");
        inputFiles.add(".\\ressources\\alphabet.txt");
        inputFiles.add(".\\ressources\\test.txt");

        System.out.println("(Logiciel 1) Lancement de l'analyseur de corpus.");
        new Analyzer(inputFiles);   // Run : Input -> ... -> Output
        System.out.println("(Fin du Logiciel 1) Au revoir !");
    }
}
