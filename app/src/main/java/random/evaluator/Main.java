package random.evaluator;

import java.io.IOException;
import java.util.List;

public class Main {
    public static void main(String[] args) {
        System.out.println("(Logiciel 2) Lancement de l'évaluateur de disposition de clavier.");
        NGramProcessor processor = new NGramProcessor();
        
        try {
            String filePath = "ressources/ngrams.csv";
            List<String> nGrams = processor.getNGramsFromCSV(filePath);

            Movement movement = new Movement(nGrams);
            movement.evaluator();
            System.out.println("(Fin du Logiciel 2) Au revoir !");
        } catch (IOException e) {
            System.err.println("Erreur lors de la lecture en liste du ngrams.csv : " + e.getMessage());
        }
    }
}