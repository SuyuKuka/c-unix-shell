import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public final class FileReader {   // Reader
    private final List<Path> listFile;
    private final HashMap<String, Integer> listNGram  = new HashMap<>();

    // Constructeur + lance lecture du fichier et création de la liste
    public FileReader(List<Path> listCorpus) {
        this.listFile = listCorpus;
        addInList();
    }


    // Gestion des n-grammes dans la ligne, et fusion pour parallélisme
    public void addNGram(int n, String line, ConcurrentHashMap<String, Integer> fusion) {
        for (int i=0; i<line.length(); i++){
            if (i + n-1 < line.length()) {
                String gram = line.substring(i, i+n);
                fusion.merge(gram, 1, Integer::sum);
            }
        }
    }

    // Tant qu'on ne dépasse pas taille du texte / longueur donnée, on coupe petit à petit la chaîne du texte puis on renvoie cette partie coupée.
    private List<String> corpusCutter(List<String> text) {
        List<String> cut = new ArrayList<>();
        do {
            String str = "";
            for (int i=0; i<50; i++) {  // taille du 'i<n' modulable selon les besoins des tailles de coupure du corpus
                if (i >= text.size()) { break; }
                str = text.get(i) + " " + str;
                text.remove(text.get(i));
            }
            cut.add(str);
        } while (!text.isEmpty());
        return cut;
    }

    /**
     * Traitement d'une liste de fichiers pour en extraire les n-grammes en parallèle (threads).
     * 
     * On lit un fichier ligne par ligne, on le nettoie puis on compte ses n-grammes dans une ConcurrentHashMap.
     * Dès que tous les fichiers analysés, on fusionne les résultats dans la liste listNGram.
     * Utilisation d'un pool de threads pour accélérer le traitement & gérer les conflits avec ConcurrentHashMap.
    */
    public void addInList() {
        ExecutorService executorService = Executors.newFixedThreadPool(4); // 4 ?
        ConcurrentHashMap<String, Integer> fusion = new ConcurrentHashMap<>();
        for (Path file : listFile) {
            executorService.submit(() -> {
                try {
                    List<String> text = new ArrayList<>(Files.readAllLines(Paths.get(file + "")));
                    text = corpusCutter(text);
                    for (String cut : text) {
                        addNGram(1, cut, fusion);
                        addNGram(2, cut, fusion);
                        addNGram(3, cut, fusion);
                    }
                } catch (IOException e) {
                    System.err.println("Une erreur est survenue lors de la lecture des fichiers : " + e.getMessage());
                }
            });
        }

        executorService.shutdown();
        try {
            boolean ended = executorService.awaitTermination(10, TimeUnit.SECONDS); // Wait la fin de toutes les tâches, boolean jamais use ?
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.err.println("Interruption thread : " + e.getMessage());
        }
        // Synchronisation des résultats pour assurer l'intégrité des données
        synchronized (listNGram) {
            for (Map.Entry<String, Integer> entry : fusion.entrySet()) {
                listNGram.merge(entry.getKey(), entry.getValue(), Integer::sum);
            }
        }
    }


    /* Getters */
    public HashMap<String, Integer> getListNGram() {
        return listNGram;
    }
}
