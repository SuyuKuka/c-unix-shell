package random.analyzer;

import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class App {

    private static Path getWorkingDirectoryPath(String relativePath) {  // Chemin basé sur le répertoire de travail actuel
        return Paths.get(System.getProperty("user.dir")).resolve(relativePath);
    }

    public static void main(String[] args) {
        final List<String> inputFiles = new ArrayList<>();

        inputFiles.addAll(Arrays.asList(args)); // Si pas de switch pour présence d'options, pour tout mettre (args donnés au main)
        /* Appels fonctionnant avec le run main classique */
        //inputFiles.add(".\\ressources\\poésie.txt");
        //inputFiles.add(".\\ressources\\alphabet.txt");
        //inputFiles.add(".\\ressources\\test.txt");

        /* Appels fonctionnant pour le ./gradlew run */
        //inputFiles.add(getWorkingDirectoryPath("ressources/poésie.txt").toString());
        inputFiles.add(getWorkingDirectoryPath("ressources/alphabet.txt").toString());
        //inputFiles.add(getWorkingDirectoryPath("ressources/corpus.txt").toString());

        System.out.println("(Logiciel 1) Lancement de l'analyseur de corpus.");
        new Analyzer(inputFiles);   // Run : Input -> ... -> Output
        System.out.println("(Fin du Logiciel 1) Au revoir !");
    }
}
