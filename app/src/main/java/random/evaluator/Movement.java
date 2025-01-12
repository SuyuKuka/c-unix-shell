package random.evaluator;

import java.util.ArrayList;
import java.util.List;


public class Movement { 
    private double totalScore = 100.0;

    private ArrayList<String> characters = new ArrayList<String>();
    private ArrayList<String> finger = new ArrayList<String>();
    private ArrayList<int[]> positions = new ArrayList<int[]>();

    public Movement(List<String> ngrams) {
        start(ngrams);
    }

    /**
     * Initialisation du mouvement
     * @param ngrams la liste des n-grams
     * A chaque ngram, on decompose le ngram en caracteres,
     * on traduit les caracteres en leur equivalent sur le clavier,
     * on ajoute les informations de position, de doigt et de caractere
     * a la liste correspondante,
     * on calcule le score du mouvement
     * et on reinitialise le clavier et les valeurs des listes
     * pour decomposer le ngram suivant.
     */
    private void start(List<String> ngrams) {
        Keyboard keyboard = new Keyboard();
        for (String ngram : ngrams) {
            keyboard.decomposeAndTranslateNgram(ngram);
            characters.addAll(keyboard.getCharacters());
            finger.addAll(keyboard.getFinger());
            positions.addAll(keyboard.getPositions());
            calculateScore();
            keyboard.reset();
            characters.clear();
            finger.clear();
            positions.clear();
        }
    }

    /**
     * Calcule le score du mouvement en fonction de la taille du n-gramme.
     * Pour chaque taille de mouvement (1, 2 ou 3 caractères), le score est ajusté
     * en fonction des poids associés aux caractéristiques du mouvement.
     * Réduit ou augmente le score total en fonction des mouvements calculés 
     * pour évaluer le clavier sur 100. 
     * Gère les n-grammes jusqu'à une longueur de 3, affiche une erreur si la taille
     * dépasse cette limite.
     */

    private void calculateScore() {
        // "Movement" a une seule touche
        if (characters.size() == 1) {

            System.out.print("Taille 1 : ");
            for (int i = 0; i < characters.size(); i++) {
                System.out.print(characters.get(i));
            }
            System.out.println("");

            int[] position = positions.get(0);
            String[] fingerUsed = split(finger.get(0)); 
            String fingerSt = fingerUsed[0];
            String hand = fingerUsed[1];

            double handWeight = getHandWeight(hand);
            double positionWeight = getPositionWeight(position);
            double fingerWeight = getFingerWeight(fingerSt);
            totalScore = totalScore - (handWeight + positionWeight + fingerWeight);
            System.out.println("Poids du mouvement (Taille 1) : " + (handWeight + positionWeight + fingerWeight));
        }
        // Mouvement de longueur 2
        else if (characters.size() == 2) {

            System.out.print("Taille 2 : ");
            for (int i = 0; i < characters.size(); i++) {
                System.out.print(characters.get(i));
            }
            System.out.println("");

            int[] pos1 = positions.get(0);
            int[] pos2 = positions.get(1);

            String[] fingerUsed1 = split(finger.get(0)); 
            String fingerSt1 = fingerUsed1[0];
            String hand1 = fingerUsed1[1];

            String[] fingerUsed2 = split(finger.get(1)); 
            String fingerSt2 = fingerUsed2[0];
            String hand2 = fingerUsed2[1];

            double sameFinger = getSameFinger(pos1, pos2);
            double scissors = getScissors(pos1, pos2, hand1, hand2);
            double lsb = getLateralStretch(pos1, pos2, fingerSt1, fingerSt2);
            double alternance = getAlternance(hand1, hand2);
            double roulement = getRoulement(pos1, pos2, hand1, hand2, fingerSt1, fingerSt2);
            totalScore = totalScore - (sameFinger + scissors + lsb) + (alternance + roulement);
            System.out.println("Poids du mouvement (Taille 2) : " + (sameFinger + scissors + lsb));
        }
        // Movement de longueur 3
        else if (characters.size() == 3) {

            System.out.print("Taille 3 : ");
            for (int i = 0; i < characters.size(); i++) {
                System.out.print(characters.get(i));
            }
            System.out.println("");

            int[] pos1 = positions.get(0);
            int[] pos2 = positions.get(1);
            int[] pos3 = positions.get(2);

            String[] fingerUsed1 = split(finger.get(0)); 
            String fingerSt1 = fingerUsed1[0];
            String hand1 = fingerUsed1[1];

            String[] fingerUsed2 = split(finger.get(1)); 
            String fingerSt2 = fingerUsed2[0];
            String hand2 = fingerUsed2[1];

            String[] fingerUsed3 = split(finger.get(2)); 
            String fingerSt3 = fingerUsed3[0];
            String hand3 = fingerUsed3[1];

            double redirections = getRedirections(pos1, pos2, pos3, hand1, hand2, hand3, fingerSt1, fingerSt2, fingerSt3);
            double skipGrammes = getSkipGrammes(hand1, hand2, hand3, fingerSt1, fingerSt2, fingerSt3);
            totalScore = totalScore - (skipGrammes + redirections);
            System.out.println("Poids du mouvement (Taille 3) : " + (skipGrammes + redirections));
        } else {
            String ngram = "";
            for (int i = 0; i < characters.size(); i++) {
                ngram += characters.get(i);
            }
            System.err.println("Le ngram " + ngram + " est trop long (cas gérer jusqu'à 3 grammes)");
        }
    }

    /**
     * @param fingerUsed
     * @return un tableau de 2 string, le premier élément est le doigt (en minuscule), le deuxième élément est la main (en minuscule)
     */
    public String[] split(String fingerUsed) {
        String[] st = fingerUsed.split("_");
        st[0] = st [0].toLowerCase();
        st[1] = st[1].toLowerCase();
        return st;
    }

    // "Mouvements" à une seule touche

    /**
     * @param finger
     * @return le poids de la main (1.0 pour la main droite, 1.1 pour la main gauche)
     */
    private double getHandWeight(String finger) {
        if (finger.equals("left")) {
            return 1.1;
        } else {
            return 1.0;
        }
    }
    
    /**
     * @param position
     * @return le poids de la position du doigt sur le clavier (en fonction de la distance à la position 1)
     */
    private double getPositionWeight(int[] position) {
        double positionWeight = 0.0;
        if (position[0] == 1) {
            return 0.0;
        } else if (position[0] == 2 || position[0] == 0) {
            positionWeight += 0.5;
        } else if (position[0] == 4) {
            return 0.2;
        } else {
            positionWeight += 0.8;
        }

        if (7 > position[1] && position[1] > 3) {
            positionWeight += 0.5;
        } else {
            positionWeight += 0.8;
        }
        return positionWeight;
    }
    
    /**
     * @param fingerUsed
     * @return le poids du doigt
     * 1.4 pour l'auriculaire, 1.2 pour l'annulaire, 0.8 pour le pouce, 1.0 pour les autres
     */
    private double getFingerWeight(String fingerUsed) {
        if (fingerUsed.equals("pinky")) {
            return 1.4;
        } else if (fingerUsed.equals("ring")) {
            return 1.2;
        } else if (fingerUsed.equals("thumb")) {
            return 0.8;
        }
        return 1.0;
    }

    // Mouvements de longueur 2

    /**
     * @param pos1
     * @param pos2
     * @return 2.0 si les deux caracteres sont sur la meme colonne et differents lignes sinon 0.0
     */
    private double getSameFinger(int[] pos1, int[] pos2) {
        if (pos1[1] == pos2[1] && pos1[0] != pos2[0]) {
            return 2.0; 
        }
        return 0.0;
    }

    /**
     * @param pos1
     * @param pos2
     * @param hand1
     * @param hand2
     * @return 2.0 si les deux caracteres sont sur la meme main mais pas sur la meme ligne
     *         et pas sur la meme colonne, 0.0 sinon
     */
    private double getScissors(int[] pos1, int[] pos2, String hand1, String hand2) {
        boolean sameHand = hand1.equals(hand2);
        if (sameHand && pos1[0] != pos2[0]) {
            if (pos1[1] != pos2[1]) {
                return 2.0;
            }
        }
        return 0.0;
    }

    /**
     * @param pos1 
     * @param pos2 
     * @param finger1 
     * @param finger2 
     * @return 2.0 si les deux caracteres ne sont pas sur la meme colonne mais qui utilise le meme doigt
     */
    private double getLateralStretch(int[] pos1, int[] pos2, String finger1, String finger2) {
        if (pos1[1] != pos2[1] && finger1.equals(finger2)) {
            return 2.0;
        }
        return 0.0;
    }

    /**
     * @param hand1 
     * @param hand2 
     * @return 1.5 si les deux caracteres sont sur des mains differentes, 0.0 sinon
     */
    private double getAlternance(String hand1, String hand2) {
        if (!hand1.equals(hand2)) {
            return 1.2; 
        }
        return 0.0;
    }
    
    /**
     * @param pos1 
     * @param pos2 
     * @param hand1 
     * @param hand2 
     * @param finger1 
     * @param finger2 
     * @return 1.5 si les deux caracteres sont sur la meme main mais qui utilise un doigt different
     *         et que le mouvement se fait de gauche a droite pour la main gauche et de droite a gauche
     *         pour la main droite, 0.0 sinon
     */
    private double getRoulement(int[] pos1, int[] pos2, String hand1, String hand2, String finger1, String finger2) {
        if (hand1.equals(hand2) && !finger1.equals(finger2)) {
            if (pos1[1] < pos2[1] && hand1.equals("left")
            || pos1[1] > pos2[1] && hand1.equals("right")) {
                return 3.0;
            }
        }
        return 0.0;
    }

    // Mouvement de longueur 3

    /**
     * @param pos1 
     * @param pos2 
     * @param pos3 
     * @param hand1 
     * @param hand2 
     * @param hand3 
     * @param finger1 
     * @param finger2 
     * @param finger3 
     * @return 3.0 si les trois caracteres sont sur la meme main,
     *         et que la position fais un changement de direction,
     *         (intérieur extérieur intérieur ou inversement) et
     *         si l'index n'a pas été utilisé alors il est considéré comme le pire mouvement
     *         0.0 sinon.
     */
    private double getRedirections(int[] pos1, int[] pos2, int[] pos3, String hand1, String hand2, String hand3, String finger1, String finger2, String finger3) {
        if (hand1.equals(hand2) && hand2.equals(hand3)) {
            if (pos1[1] > pos2[1] && pos2[1] < pos3[1]
                || pos1[1] < pos2[1] && pos2[1] > pos3[1]) {
                if (!finger1.equals("index") && !finger2.equals("index") && !finger3.equals("index")) {
                    return 5.0;
                }
                return 3.0;
            }
        }
        return 0.0;
    }

    /**
     * @param hand1 
     * @param hand2 
     * @param hand3 
     * @param finger1 
     * @param finger2 
     * @param finger3 
     * @return 3.0 si les trois caracteres utilisent le meme doigt mais que la deuxieme main est differente
     *         du premier et du troisieme, 0.0 sinon.
     */
    private double getSkipGrammes(String hand1, String hand2, String hand3, String finger1, String finger2, String finger3) {
        if (finger1.equals(finger2) && finger2.equals(finger3)) {
            if (hand1.equals(hand3) && !hand2.equals(hand3)) {
                return 3.0; 
            }
        }
        return 0.0;
    }

    /**
     * @return le score total
     */
    public double getTotalScore() {
        return totalScore;
    }
}
