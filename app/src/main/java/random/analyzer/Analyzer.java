package random.analyzer;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class Analyzer {
    private final List<String> inputFiles;

    // Constructeur + lance directement l'analyzer
    public Analyzer(List<String> inputFiles) {
        this.inputFiles = List.copyOf(inputFiles);
        run();
    }

    // Méthode principale qui run le logiciel 1.
    private void run() {
        final Set<String> allInputFiles = new LinkedHashSet<>(inputFiles);

        // Création puis démarrage des threads
        final List<Thread> threadsList = new ArrayList<>();
        for (final String inputFile : allInputFiles) {
            Thread t = new Thread(() -> parseFile(inputFile), inputFile);
            threadsList.add(t);
            t.start();
        }
        for (Thread thread : threadsList) {
            try {
                thread.join(Duration.ofMinutes(10));
            } catch (InterruptedException e) {
                System.err.println("Une erreur est survenue lors du parsage du fichier " + thread.getName() + " (gestion des threads).");
            }
        }
    }


    // Parse le fichier 'inputFile' puis crée son fichier csv.
    private void parseFile(String inputFile) {
        try {
            // Vérification des contrats
            verifyInputFileExists(inputFile);   // Etape 1
            System.out.println("Etape 1 faite...");
            final String outputFile = changeFileExtensionToCSV(inputFile);  // Etape 2
            System.out.println("Etape 2 faite...");
            /* 
                Cheminement à comprendre : fichier.txt input -> String -> Map -> fichier.txt output
            */
            final String content = Parser.parse(inputFile);     // Etape 3, fichier.txt input -> String
            System.out.println("Etape 3 faite...");
            final Map<Character, MutableInt> map = mapStringContent(content);   // Etape 4, String -> HashMap
            System.out.println("Etape 4 faite, j'appelle fromMapToCSV !");
            fromMapToCSV(map, outputFile);      // Etape finale, HashMap -> fichier.txt output (csv)
        } catch (FileNotFoundException e) {
            System.err.println("Erreur : le fichier " + e.getMessage() + " n'existe pas. ");
        } catch (SecurityException | IOException e) {
            System.err.println("Une erreur est survenue lors de l'écriture/lecture du fichier. " + e.getClass().getName() + " : " + e.getMessage());
        }
    }

    // Renvoie une FileNotFoundException si le path vers le fichier inputFile ne trouve rien.
    private void verifyInputFileExists(String inputFile) throws FileNotFoundException {
        if (!new File(inputFile).exists()) {
            throw new FileNotFoundException(inputFile);
        }
    }

    // Renvoie une map créée en comptant les occcurences de tous les caractères du 'content' donné.
    private Map<Character, MutableInt> mapStringContent(String content) {
        final Map<Character, MutableInt> map = new LinkedHashMap<>();
        for (int i = 0; i < content.length(); i++) {
            final char c = content.charAt(i);
            final MutableInt count = map.get(c);
            if (count == null) {
                map.put(c, new MutableInt());
            } else {
                count.increment();
            }
        }
        return map;
    }

    // Création du fichier 'outputFile' (format .csv) à partir de la map donnée.
    private void fromMapToCSV(Map<Character, MutableInt> map, String outputFile) throws SecurityException, IOException {
        try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(outputFile), StandardCharsets.UTF_8))) {
            final long totalOccurences = map.values().stream().mapToInt(MutableInt::get).sum();
            writer.write("caractère;nb_occurence;fréquence\n");

            for (final Map.Entry<Character, MutableInt> entry : map.entrySet()) {
                // caractère
                final String key = String.valueOf(entry.getKey()).replace("\n","\\n").replace("\t", "\\t"); // empêche l'échappement des caractères spéciaux, retour à la ligne et tabulation
                // nb_occurence
                final long occurences = entry.getValue().get();
                // fréquence
                double frequency = (double) occurences / totalOccurences * 100;

                writer.write(key + ";" + occurences + ";" + String.format("%.2f", frequency) + "%\n");
            }
        }
    }
    

    // Renvoie le path d'un fichier en ayant changé son extension en csv.
    private static String changeFileExtensionToCSV(String filePath) {
        final int dotIndex = filePath.lastIndexOf('.');
        if(dotIndex == -1) {
            return filePath + ".csv";
        }
        return filePath.substring(0, dotIndex) + ".csv";
    }

    // Classe pour utiliser les int de manière + dynamique, peut faire sans ?
    private static final class MutableInt {
        private int value = 1;

        private int get() {
            return value;
        }
        private void increment() {
            ++value;
        }
    }
}
