package random.analyzer;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Parser {   // final ? Pourrait être commune à logiciel_1 et logiciel_2
    
    private Parser() { }

    /* Lit le fichier 'filePath' puis renvoie son contenu sous la forme d'un String. */
    public static String parse(String filePath) throws SecurityException, FileNotFoundException, IOException {
        StringBuilder builder = new StringBuilder();
        try(BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(filePath), StandardCharsets.UTF_8))) {
            int c;
            while ((c = reader.read()) != -1) {
                builder.append(String.valueOf((char) c));
            }
        }
        return builder.toString();
    }
}
