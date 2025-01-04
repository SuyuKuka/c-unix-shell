import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public final class Analyzer { // Checker, rajouter Interfaces ?
    private final Path ressources = Paths.get("");  // Chemin vers folder qui va contenir le tout (result, corpus,  log  si analyse pas 2 fois même texte)
    private final Path corpusDir = Paths.get("");   // chemin vers le folder contenant tous les corpus à analyser
    private final Path filePath = Paths.get(""); // chemin vers le fichier résultat du logiciel, un CSV ou un JSON
    private final List<Path>  listCorpus;
    // rajouter variable Path pour log ?

    // Constructeur
    public Analyzer() {
        this.listCorpus = listFiles(corpusDir);
    }

    // Initialisation de l'analyseur
    public void init() {
        ifNotCreateDir(ressources);
        ifNotCreateDir(corpusDir);
        ifNotCreateFile(filePath);
    }


    // Récupération de la liste de fichiers dans le répertoire 'dir'.
    public List<Path> listFiles(Path dir) {
        try (Stream<Path> stream = Files.list(dir)) {
            return stream.filter(file -> !Files.isDirectory(file))
            .collect(Collectors.toList());
        } catch (IOException e) {
            System.err.println("Une erreur est survenue lors de la création du répertoire : "  +  e.getMessage());
            return new ArrayList<>();
        }
    }
    

    /* Méthodes de vérification des fichiers */
    public void ifNotCreateDir(Path path) {
        if (!Files.exists(path) || !Files.isDirectory(path)) {
            try {
                Files.createDirectory(path);
                System.out.println("Le répertoire pour " + path + " a été créé avec succès.");
            } catch (IOException e) {
                System.err.println("Une erreur est survenue lors de la création du répertoire : "  +  e.getMessage());
            }
        }
    }
    public void ifNotCreateFile(Path path) {
        if (!Files.exists(path)) {
            try {
                Files.createFile(path);
                System.out.println("Le fichier pour " + path + " a été créé avec succès.");
            } catch (IOException e) {
                System.err.println("Une erreur est survenue lors de la création du fichier : "  +  e.getMessage());
            }
        }
    }

    
    /* Getters */
    public List<Path> getListTexts() {
        return listCorpus;
    }
    public Path getFilePath() {
        return filePath;
    }
}
