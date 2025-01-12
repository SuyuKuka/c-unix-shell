package random.evaluator;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import random.evaluator.Keyboard.Position;

public class Movement { 
    private double totalScore = 100.0;

    private List<String> ngrams;
    private List<String> characters;
    private List<Finger> fingers;
    private List<Hand> hands;
    private List<Position> positions;

    private Map<String, Double> percent = new HashMap<>();
    private Map<String, Double> scores = new HashMap<>();

    public Movement(List<String> ngrams) {
        this.ngrams = new ArrayList<>(ngrams);
    }

    public enum Hand {
        LEFT,
        RIGHT
    }
    
    public enum Finger {
        THUMB,
        INDEX,
        MIDDLE,
        RING,
        PINKY
    }

    /**
     * Initialisation du keyboard
     * A chaque ngram, on decompose le ngram en caracteres,
     * on traduit les caracteres en leur equivalent sur le clavier,
     * on ajoute les informations de position, de doigt, de main et de caractere
     * a la liste correspondante,
     * on calcule le score du mouvement
     * et on reinitialise les valeurs des listes
     * pour decomposer le ngram suivant.
     * On affiche le score final.
     */
    public void evaluator() {
        Keyboard keyboard = new Keyboard();
        init();
        for (String ngram : ngrams) {
            keyboard.decomposeAndTranslateNgram(ngram);
            characters = keyboard.getCharacters();
            fingers = keyboard.getFingers();
            hands = keyboard.getHands();
            positions = keyboard.getPositions();
            try {
                calculateScore();
            } catch (InvalidNgramLengthException e) {
                e.printStackTrace();
            }
            keyboard.reset();
        }
        valuateScore();
        printFinalScore();
    }

    /**
     * Initialisation des scores
     * On met a zero tous les scores
     */
    private void init() {
        scores.put("handWeight", 0.0);
        scores.put("positionWeight", 0.0);
        scores.put("fingerWeight",0.0);
        scores.put("sameFinger", 0.0);
        scores.put("scissors", 0.0);
        scores.put("lsb", 0.0);
        scores.put("alternance", 0.0);
        scores.put("roulement", 0.0);
        scores.put("redirections", 0.0);
        scores.put("skipGrammes", 0.0);
    }

    /**
     * Valuation du score du mouvement
     * On met a jour le score total en fonction des pourcetages qui lui sont attribues
     * et des scores des caracteristiques du mouvement.
     * On affiche le score final.
     */
    private void valuateScore() { 
        percent.put("handWeight", 5.0);
        percent.put("positionWeight", 10.0);
        percent.put("fingerWeight", 10.0);
        percent.put("sameFinger", 15.0);
        percent.put("scissors", 15.0);
        percent.put("lsb", 15.0);
        percent.put("alternance", 20.0);
        percent.put("roulement", 20.0);
        percent.put("redirections", 15.0);
        percent.put("skipGrammes", 15.0);

        for (String key : percent.keySet()) {
            double score = scores.get(key);
            scores.put(key, Math.min(score, 20.0));
        }

        for (String key : scores.keySet()) {
            totalScore -= (scores.get(key) / 20) * percent.get(key) * totalScore / 100.0;
        }

        totalScore = Math.max(0.0, Math.min(totalScore, 100.0));
    }

    /**
     * Calcule le score du mouvement en fonction de la taille du n-gramme.
     * Pour chaque taille de mouvement (1, 2 ou 3 caractères), le score est ajusté
     * en fonction des poids associés aux caractéristiques du mouvement.
     * Réduit ou augmente le score total en fonction des mouvements calculés 
     * pour évaluer le clavier sur 100. 
     * Gère les n-grammes jusqu'à une longueur de 3, affiche une erreur si la taille
     * dépasse cette limite.
     * @throws InvalidNgramLengthException 
    */
    private void calculateScore() throws InvalidNgramLengthException {
        // "Movement" a une seule touche
        if (characters.size() == 1) {
            Position position = positions.get(0);
            Finger finger = fingers.get(0);
            Hand hand = hands.get(0);

            scores.put("handWeight", scores.get("handWeight") + isHandWeight(hand));
            scores.put("positionWeight", scores.get("positionWeight") + isPositionWeight(position));
            scores.put("fingerWeight", scores.get("fingerWeight") + isFingerWeight(finger));
        }
        // Mouvement de longueur 2
        else if (characters.size() == 2) {
            Position pos1 = positions.get(0);
            Position pos2 = positions.get(1);

            Finger finger1 = fingers.get(0);
            Finger finger2 = fingers.get(1);

            Hand hand1 = hands.get(0);
            Hand hand2 = hands.get(1);

            scores.put("sameFinger", scores.get("sameFinger") + isSameFinger(pos1, pos2));
            scores.put("scissors", scores.get("scissors") + isScissors(pos1, pos2, hand1, hand2));
            scores.put("lsb", scores.get("lsb") + isLateralStretch(pos1, pos2, finger1, finger2));
            scores.put("alternance", scores.get("alternance") + isAlternance(hand1, hand2));
            scores.put("roulement", scores.get("roulement") + isRoulement(pos1, pos2, hand1, hand2, finger1, finger2));
        }
        // Movement de longueur 3
        else if (characters.size() == 3) {
            Position pos1 = positions.get(0);
            Position pos2 = positions.get(1);
            Position pos3 = positions.get(2);

            Finger finger1 = fingers.get(0);
            Finger finger2 = fingers.get(1);
            Finger finger3 = fingers.get(2);

            Hand hand1 = hands.get(0);
            Hand hand2 = hands.get(1);
            Hand hand3 = hands.get(2);

            scores.put("redirections", scores.get("redirections") + isRedirections(pos1, pos2, pos3, hand1, hand2, hand3, finger1, finger2, finger3));
            scores.put("skipGrammes", scores.get("skipGrammes") + isSkipGrammes(hand1, hand2, hand3, finger1, finger2, finger3));
        } else {
            String ngram = "";
            for (int i = 0; i < characters.size(); i++) {
                ngram += characters.get(i);
            }
            throw new InvalidNgramLengthException("Le ngram " + ngram + " est trop long (cas gérer jusqu'à 3 grammes)");
        }
    }

    // "Mouvements" à une seule touche

    /**
     * @param hand
     * @return le poids de la main (1.0 pour la main droite, 1.1 pour la main gauche)
     */
    private double isHandWeight(Hand hand) {
        if (hand == Hand.LEFT) {
            return 1.1;
        } else {
            return 1.0;
        }
    }
    
    /**
     * @param position
     * @return le poids de la position du doigt sur le clavier (en fonction de la distance à la position 1)
     */
    private double isPositionWeight(Position position) {
        double positionWeight = 0.0;
        if (position.row() == 1) {
            return 0.0;
        } else if (position.row() == 2 || position.row() == 0) {
            positionWeight += 0.5;
        } else if (position.row() == 4) {
            return 0.2;
        } else {
            positionWeight += 0.8;
        }

        if (7 > position.column() && position.column() > 3) {
            positionWeight += 0.5;
        } else {
            positionWeight += 0.8;
        }
        return positionWeight;
    }
    
    /**
     * @param finger
     * @return le poids du doigt
     * 1.4 pour l'auriculaire, 1.2 pour l'annulaire, 0.8 pour le pouce, 1.0 pour les autres
     */
    private double isFingerWeight(Finger finger) {
        if (finger == Finger.PINKY) {
            return 1.4;
        } else if (finger == Finger.RING) {
            return 1.2;
        } else if (finger == Finger.THUMB) {
            return 0.8;
        }
        return 1.0;
    }

    // Mouvements de longueur 2

    /**
     * @param pos1
     * @param pos2
     * @return 2.0 si les deux caracteres sont sur la meme colonne et differents lignes 
     *          ou si ils sont sur la meme position, sinon 0.0
     */
    private double isSameFinger(Position pos1, Position pos2) {
        if (pos1.column() == pos2.column() && pos1.row() != pos2.row()
        || pos1.row() == pos2.row() && pos1.column() == pos2.column()) {
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
    private double isScissors(Position pos1, Position pos2, Hand hand1, Hand hand2) {
        if (hand1 == hand2 && pos1.row() != pos2.row()) {
            if (pos1.column() != pos2.column()) {
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
    private double isLateralStretch(Position pos1, Position pos2, Finger finger1, Finger finger2) {
        if (pos1.column() != pos2.column() && finger1 == finger2) {
            return 2.0;
        }
        return 0.0;
    }

    /**
     * @param hand1 
     * @param hand2 
     * @return 1.5 si les deux caracteres sont sur des mains differentes, 0.0 sinon
     */
    private double isAlternance(Hand hand1, Hand hand2) {
        if (hand1 != hand2) {
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
    private double isRoulement(Position pos1, Position pos2, Hand hand1, Hand hand2, Finger finger1, Finger finger2) {
        if (hand1 == hand2 && finger1 != finger2) {
            if (pos1.column() < pos2.column() && hand1 == Hand.LEFT
            || pos1.column() > pos2.column() && hand1 == Hand.RIGHT) {
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
    private double isRedirections(Position pos1, Position pos2, Position pos3, Hand hand1, Hand hand2, Hand hand3, Finger finger1, Finger finger2, Finger finger3) {
        if (hand1 == hand2 && hand2 == hand3) {
            if (pos1.column() > pos2.column() && pos2.column() < pos3.column()
                || pos1.column() < pos2.column() && pos2.column() > pos3.column()) {
                if (finger1 != Finger.INDEX && finger2 != Finger.INDEX && finger3 != Finger.INDEX) {
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
    private double isSkipGrammes(Hand hand1, Hand hand2, Hand hand3, Finger finger1, Finger finger2, Finger finger3) {
        if (finger1 == finger2 && finger2 == finger3) {
            if (hand1 == hand2 && hand2 != hand3) {
                return 3.0; 
            }
        }
        return 0.0;
    }

    /**
     * Print le score final
     */
    private void printFinalScore() {
        System.out.println("Score total : " + totalScore + "/100.");
    }

    private class InvalidNgramLengthException extends Exception {
        public InvalidNgramLengthException(String message) {
            super(message);
        }
    }
}
