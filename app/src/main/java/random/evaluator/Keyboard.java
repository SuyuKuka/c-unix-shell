package random.evaluator;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

public class Keyboard {
    private ArrayList<String> characters = new ArrayList<String>();
    private ArrayList<String> finger = new ArrayList<String>();
    private ArrayList<int[]> positions = new ArrayList<int[]>();

    /**
     * Reinitialise les listes des caracteres, des doigts et des positions
     * pour pouvoir recommencer a decomposer un autre ngram
     */
    public void reset() {
        characters = new ArrayList<String>();
        finger = new ArrayList<String>();
        positions = new ArrayList<int[]>();
    }

    /**
     * @param ngram On prend le ngram en parametre et on le decompose
     * On prend chaque lettre du ngram et on la traduit
     * Apres la traduction du ngram on appelle la fonction ngramToKeyboard
     */
    public void decomposeAndTranslateNgram(String ngram) {
        for (int i = 0; i < ngram.length(); i++) {
            String currentChar = String.valueOf(ngram.charAt(i));
            translateNgram(currentChar);
        }
        ngramToKeyboard(characters);
    }

    /**
     * @param ngram 
     * Prend un ngram (un seul caractere) et le traduit en son equivalent sur le clavier. 
     */
    private void translateNgram(String ngram) {
        if (digitTranslation(ngram)) {
            return;
        } else if (symbolsTranslation(ngram)) {
            return;
        } else if (specialCharTranslation(ngram)) {
            return;
        } else {
            if (Character.isUpperCase(ngram.charAt(0))) {
                characters.add("shift"); characters.add(ngram.toLowerCase());
                return;
            } else {
                characters.add(ngram);
                return;
            }
        }
    }

    /**
     * @param digit
     * @return true si le caractere a ete traduit, false sinon
     * Prend un ngram qui est un chiffre et le traduit en son equivalent sur le clavier.
     */
    private boolean digitTranslation(String digit) {
        switch (digit) {
            case "1": characters.add("shift"); characters.add("&"); return true;
            case "2": characters.add("shift"); characters.add("é"); return true;
            case "3": characters.add("shift"); characters.add("\""); return true;
            case "4": characters.add("shift"); characters.add("'"); return true;
            case "5": characters.add("shift"); characters.add("("); return true;
            case "6": characters.add("shift"); characters.add("-"); return true;
            case "7": characters.add("shift"); characters.add("è"); return true;
            case "8": characters.add("shift"); characters.add("_"); return true;
            case "9": characters.add("shift"); characters.add("ç"); return true;
            case "0": characters.add("shift"); characters.add("à"); return true;
            default: return false;
        }
    }

    /**
     * @param st
     * @return true si le caractere a ete traduit, false sinon
     * Prend un ngram qui est un symbole et le traduit en son equivalent sur le clavier.
     */
    private boolean symbolsTranslation (String st) {
        switch (st) {
            case " ": characters.add("space"); return true;

            case "°": characters.add("shift"); characters.add(")"); return true;
            case "+": characters.add("shift"); characters.add("="); return true;
            case "¨": characters.add("shift"); characters.add("^"); return true;
            case "£": characters.add("shift"); characters.add("$"); return true;
            case "%": characters.add("shift"); characters.add("ù"); return true;
            case "µ": characters.add("shift"); characters.add("*"); return true;
            case "?": characters.add("shift"); characters.add(","); return true;
            case ".": characters.add("shift"); characters.add(";"); return true;
            case "/": characters.add("shift"); characters.add(":"); return true;
            case "§": characters.add("shift"); characters.add("!"); return true;
            case ">": characters.add("shift"); characters.add("<"); return true;

            case "~": characters.add("altgr"); characters.add("é"); return true;
            case "#": characters.add("altgr"); characters.add("\""); return true;
            case "{": characters.add("altgr"); characters.add("'"); return true;
            case "[": characters.add("altgr"); characters.add("("); return true;
            case "|": characters.add("altgr"); characters.add("-"); return true;
            case "`": characters.add("altgr"); characters.add("è"); return true;
            case "\\": characters.add("altgr"); characters.add("_"); return true;
            case "^": characters.add("altgr"); characters.add("ç"); return true;
            case "@": characters.add("altgr"); characters.add("à"); return true;
            case "]": characters.add("altgr"); characters.add(")"); return true;
            case "}": characters.add("altgr"); characters.add("="); return true;
            case "¤": characters.add("altgr"); characters.add("$"); return true;
            default: return false;
        }
    }

    /**
     * @param st
     * @return true si le caractere a ete traduit, false sinon
     * Prend un ngram qui est un caractere special et le traduit en son equivalent sur le clavier.
     */
    private boolean specialCharTranslation(String st) {
        switch (st) {
            case "â": characters.add("^"); characters.add("a"); return true;
            case "Â": characters.add("^"); characters.add("shift"); characters.add("a"); return true;
            case "ä": characters.add("shift"); characters.add("^"); characters.add("a"); return true;
            case "Ä": characters.add("shift"); characters.add("^"); characters.add("shift"); characters.add("a"); return true;

            case "ê": characters.add("^"); characters.add("e"); return true;
            case "Ê": characters.add("^"); characters.add("shift"); characters.add("e"); return true;
            case "ë": characters.add("shift"); characters.add("^"); characters.add("e"); return true;
            case "Ë": characters.add("shift"); characters.add("^"); characters.add("shift"); characters.add("e"); return true;
            
            case "î": characters.add("^"); characters.add("i"); return true;
            case "Î": characters.add("^"); characters.add("shift"); characters.add("i"); return true;
            case "ï": characters.add("shift"); characters.add("^"); characters.add("i"); return true;
            case "Ï": characters.add("shift"); characters.add("^"); characters.add("shift"); characters.add("i"); return true;

            case "ô": characters.add("^"); characters.add("o"); return true;
            case "Ô": characters.add("^"); characters.add("shift"); characters.add("o"); return true;
            case "ö": characters.add("shift"); characters.add("^"); characters.add("o"); return true;
            case "Ö": characters.add("shift"); characters.add("^"); characters.add("shift"); characters.add("o"); return true;

            case "û": characters.add("^"); characters.add("u"); return true;
            case "Û": characters.add("^"); characters.add("shift"); characters.add("u"); return true;
            case "ü": characters.add("shift"); characters.add("^"); characters.add("u"); return true;
            case "Ü": characters.add("shift"); characters.add("^"); characters.add("shift"); characters.add("u"); return true;
            default: return false;
        }
    } 

    /**
     * @param ngram Liste des n-grams traduits
     * Prend un ngram et le traduit en son equivalent sur le clavier.
     * Lit le fichier de configuration du clavier (ressources/keyboard.json) 
     * pour trouver les informations de position et de doigt correspondant 
     * a chaque ngram.
     * Si le ngram n'est pas trouvé, affiche un message d'erreur.
     */
    private void ngramToKeyboard(List<String> ngram) {
        String filePath = "ressources/keyboard.json";
        try {
            String content = new String(Files.readAllBytes(Paths.get(filePath)));
            JSONObject keyboardJson = new JSONObject(content);
            JSONObject keyboard = keyboardJson.getJSONObject("keyboard");
            System.out.println("Clavier : " + keyboard.getString("layout_name"));
            JSONArray rows = keyboard.getJSONArray("rows");

            for (String ngramElement : ngram) {
                boolean found = false;
                for (int i = 0; i < rows.length(); i++) {
                    JSONObject row = rows.getJSONObject(i);
                    JSONArray keys = row.getJSONArray("keys");
                    for (int j = 0; j < keys.length(); j++) {
                        JSONObject key = keys.getJSONObject(j);
                        String value = key.getString("normal");
                        if (value.equalsIgnoreCase(ngramElement)) {
                            collectKeyInfo(key, ngramElement);
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
                if (!found) {
                    System.out.println("Caractère " + ngramElement + " non trouvé sur le clavier.");
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    /**
     * @param key JSON de la cle correspondant au ngram
     * @param ngramElement ngram a traiter
     * Prend un ngram et son JSON correspondant et extrait les informations de position, de doigt, de shift et de altgr
     * pour chaque caractere. Affiche ces informations sur le terminal.
     */
    private void collectKeyInfo(JSONObject key, String ngramElement) {
        // System.out.println("Caractère : " + ngramElement);
        // System.out.println("Position : " + key.getJSONArray("position"));
        // System.out.println("Finger : " + key.getString("finger"));
        // System.out.println("Shift : " + key.optString("shift", "N/A"));
        // System.out.println("AltGr : " + key.optString("altgr", "N/A"));
        // System.out.println("--------------------");
        finger.add(key.getString("finger"));
        positions.add(toIntArray(key.getJSONArray("position")));
    }

    /**
     * @param array un tableau JSON
     * @return un tableau d'entiers
     * Convertit un tableau JSON en un tableau d'entiers
     */
    private int[] toIntArray(JSONArray array) {
        int[] result = new int[array.length()];
        for (int i = 0; i < array.length(); i++) {
            result[i] = array.getInt(i);
        }
        return result;
    }

    /**
     * @return la liste des caracteres correspondants a chaque ngram
     * La i-eme position correspond au i-eme ngram de la liste des ngrams
     */
    public ArrayList<String> getCharacters() {
        return characters;
    }
    
    /**
     * @return la liste des doigts correspondants a chaque caractere
     * La i-eme position correspond au i-eme caractere de la liste des caracteres
     */

    public ArrayList<String> getFinger() {
        return finger;
    }

    /**
     * @return la liste des positions de chaque caractere
     * La i-eme position correspond au i-eme caractere de la liste des caracteres
     * Les positions sont au format [row_index, column_index]
     */
    public ArrayList<int[]> getPositions() {
        return positions;
    }
}
