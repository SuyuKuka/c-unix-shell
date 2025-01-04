package random.analyzer;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class App {

    public static void main(String[] args) {    // Switch si présence d'options (comme override)
        final List<String> inputFiles = new ArrayList<>();

        inputFiles.addAll(Arrays.asList(args)); // Si pas de switch, pour tout mettre (arguments donnés au main)
        inputFiles.add(".\\ressources\\corpus1.txt");
        inputFiles.add(".\\ressources\\corpus2.txt");

        // Vérification des prédicats ?
        System.out.println("Bonjour !");
        new Analyzer(inputFiles);   // Run Input -> ... -> output
        System.out.println("Au revoir !");
    }
}
