import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;

import com.opencsv.CSVWriter;   // Voir comment ajouter librairie externe pour le CSV

public class FileWriter {    // Writer
    private final Path path;
    private final HashMap<String, Integer> NGramInput;

    // Constructeur + lance le writer
    public FileWriter(Path path, HashMap<String, Integer> NGramInput) {
        this.path = path;
        this.NGramInput = NGramInput;
        write();
    }

    // Écriture des N-grammes triés par fréquence d'occurence décroissante dans un fichier CSV.
    private void write() {
        LinkedHashMap<String, Integer> sortedMap = new LinkedHashMap<>();
        ArrayList<Integer> list = new ArrayList<>();
        for(Map.Entry<String, Integer> entry : NGramInput.entrySet()) {
            list.add(entry.getValue());
        }
        Collections.sort(list);
        Collections.reverse(list);
        for (int nb : list) {
            for (Map.Entry<String, Integer> entry : NGramInput.entrySet()) {
                if (entry.getValue().equals(nb)) {
                    sortedMap.put(entry.getKey(), nb);
                }
            }
        }

        try (CSVWriter writer = new CSVWriter(
            new OutputStreamWriter(new FileOutputStream(String.valueOf(path)), StandardCharsets.UTF_8),
            ';',
            CSVWriter.DEFAULT_QUOTE_CHARACTER,
            CSVWriter.DEFAULT_ESCAPE_CHARACTER,
            CSVWriter.DEFAULT_LINE_END)) {
                
                for (String str : sortedMap.keySet()) {
                    String[] col = { str, String.valueOf(NGramInput.get(str)) };
                    writer.writeNext(col);
                }
        } catch (IOException e) {
            System.err.println("Une erreur est survenue lors de l'écriture du fichier : " + e.getMessage());
        }
    }
}
