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
import java.util.concurrent.ConcurrentHashMap;

public class Analyzer {
    private final List<String> inputFiles;
    private final Map<String, Integer> nGramMap = new ConcurrentHashMap<>(); // n-grammes

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
            Thread t = new Thread(() -> fileParser(inputFile), inputFile);
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
        // Export des n-grammes une fois que tous les threads soient terminés
        try {
            exportNGramsToCSV(".\\ressources.\\ngrams.csv");
        } catch (IOException e) {
            System.err.println("Une erreur est survenue lors de l'exportation des n-grammes : " + e.getMessage());
        }
    }

    /* 
    * Parse le fichier 'inputFile' puis crée son fichier csv.
    * Cheminement : fichier.txt input -> String -> Map -> fichier.txt output
    */
    private void fileParser(String inputFile) {
        try {   // Vérification des contrats
            verifyInputFileExists(inputFile);   // Étape 1
            System.out.println("Etape 1 faite...");

            final String outputFile = changeFileExtensionToCSV(inputFile);  // Étape 2
            System.out.println("Etape 2 faite...");
           
            final String content = Parser.parse(inputFile);     // Étape 3, fichier.txt input -> String
            System.out.println("Etape 3 faite...");

            final Map<Character, MutableInt> charCountMap = mapStringContent(content);      // Étape 4 : String -> HashMap
            // Comptage des caractères & Gestion des n-grammes
            Map<String, Integer> localNGramMap = new ConcurrentHashMap<>();
            String[] lines = content.split("\n");
            for (String line : lines) {
                if (line.trim().length() >= 1) addNGram(1, line, localNGramMap); // Caractères
                if (line.trim().length() >= 2) addNGram(2, line, localNGramMap); // Bigrammes
                if (line.trim().length() >= 3) addNGram(3, line, localNGramMap); // Trigrammes
            }
            localNGramMap.forEach((key, value) -> nGramMap.merge(key, value, Integer::sum));    // Fusionne les résultats des n-grammes localement calculés dans la map globale

            System.out.println("Etape 4 faite, j'appelle fromMapToCSV !");
            fromMapToCSV(charCountMap, outputFile);      // Étape finale, HashMap -> fichier.txt output (csv)
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

    // Renvoie le path d'un fichier en ayant changé son extension en csv.
    private static String changeFileExtensionToCSV(String filePath) {
        final int dotIndex = filePath.lastIndexOf('.');
        if(dotIndex == -1) {
            return filePath + ".csv";
        }
        return filePath.substring(0, dotIndex) + ".csv";
    }


    // Création du fichier 'outputFile' (format .csv) à partir de la map donnée.
    private void fromMapToCSV(Map<Character, MutableInt> map, String outputFile) throws SecurityException, IOException {
        try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(outputFile), StandardCharsets.UTF_8))) {
            final long totalOccurences = map.values().stream().mapToInt(MutableInt::get).sum();
            writer.write("caractère;nb_occurence;fréquence\n");

            // Trier les entrées par nb_occurence décroissant, puis par caractère (lexicographiquement) en cas d'égalité
            map.entrySet().stream().sorted((e1, e2) -> {
                int countComparison = Integer.compare(e2.getValue().get(), e1.getValue().get()); // Compare par nb_occurences
                return countComparison != 0 ? countComparison : e1.getKey().compareTo(e2.getKey()); // Ensuite compare lexicographiquement
            }).forEach(entry -> {
                try {
                    // caractère
                    final String key = String.valueOf(entry.getKey()).replace("\n", "\\n").replace("\t", "\\t"); // Éviter échappement des caractères spéciaux
                    // nb_occurence
                    final long occurences = entry.getValue().get();
                    // fréquence
                    double frequency = (double) occurences / totalOccurences * 100;

                    writer.write(key + ";" + occurences + ";" + String.format("%.2f", frequency) + "%\n");
                } catch (IOException e) {
                    System.err.println("Une erreur est survenue lors de l'écriture d'un fichier CSV : " + e.getMessage());
                }
            });
        }  catch (SecurityException | IOException e) {
            System.err.println("Une erreur est survenue lors du passage au format CSV. " + e.getClass().getName() + " : " + e.getMessage());
        }
    }



    /* Gestion des n-grammes dans l'analyseur. */

    // Génération du fichier 'ngrammes.csv' à partir du contenu de 'outputFile'.
    private void exportNGramsToCSV(String outputFile) throws IOException {
        try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(outputFile), StandardCharsets.UTF_8))) {
            writer.write("n-gramme;nb_occurrence\n");
            
            // Trie les n-grammes par longueur de key (puis ordre d'insertion si longueur égales)
            nGramMap.entrySet().stream().sorted((e1, e2) -> {
                int lengthComparison = Integer.compare(e1.getKey().length(), e2.getKey().length()); // Compare par longueur de n-gramme
                if (lengthComparison != 0) { return lengthComparison; }
                int countComparison = Integer.compare(e2.getValue(), e1.getValue());    // Compare par nb_occurrence décroissant
                if (countComparison != 0) { return countComparison; }
                // Compare par ordre lexicographique
                return e1.getKey().compareTo(e2.getKey());
            }).forEach(entry -> {
                try {
                    writer.write(entry.getKey() + ";" + entry.getValue() + "\n");
                } catch (IOException e) {
                    System.err.println("Une erreur est survenue lors de l'écriture des n-grammes dans un fichier CSV : " + e.getMessage());
                }
            });
            System.out.println("Exportation des n-grammes réussie dans " + outputFile);
        } catch (IOException e) {   // Sauvegarder dans un fichier temporaire, non nécessaire ?
            String tempFile = ".\\ressources.\\ngrams_temp_" + System.currentTimeMillis() + ".csv";
            try (BufferedWriter tempWriter = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(tempFile), StandardCharsets.UTF_8))) {
                tempWriter.write("n-gramme;nb_occurrence\n");
                
                nGramMap.entrySet().stream().sorted((e1, e2) -> {
                    int lengthComparison = Integer.compare(e1.getKey().length(), e2.getKey().length());
                    if (lengthComparison != 0) { return lengthComparison; }
                    int countComparison = Integer.compare(e2.getValue(), e1.getValue());
                    if (countComparison != 0) { return countComparison; }
                    return e1.getKey().compareTo(e2.getKey());
                    }).forEach(entry -> {
                        try {
                            tempWriter.write(entry.getKey() + ";" + entry.getValue() + "\n");
                        } catch (IOException tempEx) {
                            System.out.println("Erreur critique : une erreur est survenue lors de la création du fichier temporaire. " + tempEx.getMessage());
                        }
                    });
                System.err.println("Une erreur est survenue lors de l'écriture du fichier CSV principal. Les résultats ont été sauvegardés dans un fichier temporaire : " + tempFile);
            } catch (IOException tempEx) {
                System.err.println("Erreur critique : impossibilité de sauvegarder les résultats dans un fichier temporaire. " + tempEx.getMessage());
            }
            throw e; // Rethrow l'exception pour signaler l'échec, causerait pb par rapport au sujet ?
        }
    }

    // Gestion des n-grammes dans la ligne puis fusion pour parallélisme
    private void addNGram(int n, String line, Map<String, Integer> map) {
        if (line.length() < n) return; // Ne rien faire si la ligne est trop courte pour le n-gramme
        for (int i = 0; i <= line.length() - n; i++) {
            String nGram = line.substring(i, i + n);
            map.merge(nGram, 1, Integer::sum);
        }
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
