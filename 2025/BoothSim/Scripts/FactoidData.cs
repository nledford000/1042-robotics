using UnityEngine;
using System.Collections.Generic;

public class Factoid : MonoBehaviour
{
    [Header("Factoid Settings")]
    [Tooltip("1 = Medical, 2 = Auto, 3 = Electrical, 4 = Food")]
    public int factoidType = 1;

    [Header("Question Data (Auto-assigned)")]
    [TextArea(3, 6)]
    public string question;
    public bool correctAnswer;

    private void Start()
    {
        AssignRandomQuestion();
    }

    private void AssignRandomQuestion()
    {
        var list = GetQuestionListForType(factoidType);
        if (list == null || list.Count == 0)
        {
            question = "No questions found for this type.";
            correctAnswer = false;
            return;
        }

        var q = list[Random.Range(0, list.Count)];
        question = q.question;
        correctAnswer = q.answer;
    }

    private List<(string question, bool answer)> GetQuestionListForType(int type)
    {
        switch (type)
        {
            case 1: return medicalQuestions;
            case 2: return autoQuestions;
            case 3: return electricalQuestions;
            case 4: return foodQuestions;
            default: return null;
        }
    }

    // ======================================================
    //   QUESTION BANKS (Add 100 each for final version)
    // ======================================================

    private List<(string, bool)> medicalQuestions = new List<(string, bool)>
    {
        ("The human heart has four chambers.", true),
        ("Insulin regulates blood sugar levels.", true),
        ("The appendix is an essential organ for digestion.", false),
        ("Red blood cells carry oxygen through the body.", true),
        ("Antibiotics are effective against viruses.", false),
        ("The brain controls all voluntary movement.", true),
        ("There are 206 bones in the adult human body.", true),
        ("Your liver filters toxins from your blood.", true),
        ("A fever is always harmful and should be stopped immediately.", false),
        ("The skin is the largest organ in the human body.", true),
    };

    private List<(string, bool)> autoQuestions = new List<(string, bool)>
    {
        ("An alternator charges the car’s battery.", true),
        ("Diesel engines use spark plugs.", false),
        ("Most cars use internal combustion engines.", true),
        ("Tires should be rotated regularly for even wear.", true),
        ("The radiator helps keep the engine cool.", true),
        ("Electric vehicles require gasoline to operate.", false),
        ("Brake fluid is used in the car’s cooling system.", false),
        ("The transmission controls gear shifting.", true),
        ("A car’s battery is recharged by the alternator.", true),
        ("Motor oil lubricates moving parts in the engine.", true),
    };

    private List<(string, bool)> electricalQuestions = new List<(string, bool)>
    {
        ("Ohm’s Law relates voltage, current, and resistance.", true),
        ("Copper is a poor conductor of electricity.", false),
        ("AC stands for Alternating Current.", true),
        ("A resistor increases current flow.", false),
        ("A circuit must be closed for current to flow.", true),
        ("The unit of resistance is the Ohm.", true),
        ("A capacitor stores electrical energy.", true),
        ("Batteries produce alternating current.", false),
        ("Fuses protect circuits from overcurrent.", true),
        ("Grounding helps prevent electric shock.", true),
    };

    private List<(string, bool)> foodQuestions = new List<(string, bool)>
    {
        ("Honey never spoils.", true),
        ("Tomatoes are vegetables.", false),
        ("Vitamin C is found in oranges.", true),
        ("Boiling water freezes faster than cold water.", false),
        ("Salt preserves food by drawing out moisture.", true),
        ("Carbohydrates are the body’s main energy source.", true),
        ("Protein is mainly found in fruits.", false),
        ("Freezing food kills all bacteria instantly.", false),
        ("Olive oil is considered a healthy fat.", true),
        ("Brown eggs are healthier than white eggs.", false),
    };
}
