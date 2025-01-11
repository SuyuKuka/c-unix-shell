package random.evaluator;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class NGramProcessor {
    /* Inutile vu que gestion des n-grammes dans logiciel 1 ?
    -> Soit enlever cette classe et implémenter son contenu dans un autre fichier de l'evaluator
    -> Soit rendre la gestion des n-grammes "publique" dans une partie "common" (où il y aurait Parser.java, NGram.java,...)
    */

    /**
     * Lit un fichier CSV et retourne une liste de tous les n-grammes trouvés dans ce fichier.
     * 
     * @param filePath Chemin du fichier CSV à analyser.
     * @return Une liste de n-grammes.
     * @throws IOException En cas de problème de lecture du fichier.
     */
    public List<String> getNGramsFromCSV(String filePath) throws IOException {
        List<String> nGrams = new ArrayList<>();
        
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            String line;
            boolean isFirstLine = true; // Ignore la ligne d'en-tête
            
            while ((line = reader.readLine()) != null) {
                if (isFirstLine) {
                    isFirstLine = false;
                    continue;
                }
                // Sépare la ligne en utilisant le point-virgule comme délimiteur
                String[] parts = line.split(";");
                if (parts.length > 0) {
                    String nGram = parts[0].trim(); // Le n-gramme est la première colonne
                    nGrams.add(nGram);
                }
            }
        }
        
        return nGrams;
    }

    // Petit main juste pour test le résultat de la récupération des ngrammes en liste
    public static void main(String[] args) {
        NGramProcessor processor = new NGramProcessor();
        
        try {
            String filePath = "ressources/ngrams.csv"; // Chemin vers le .csv
            List<String> nGrams = processor.getNGramsFromCSV(filePath);
            
            // Affiche les n-grammes
            System.out.println("N-Grammes récupérés :");
            for (String nGram : nGrams) {
                System.out.print(nGram + " ");
            }
        } catch (IOException e) {
            System.err.println("Erreur lors de la lecture en liste du ngrams.csv : " + e.getMessage());
        }
    }

}
