using UnityEngine;
using TMPro;
using UnityEngine.UI;
using System.Collections;

public class FactoidQuestionSystem : MonoBehaviour
{
    [Header("UI References")]
    public GameObject questionPanel;
    public TMP_Text questionText;
    public Button trueButton;
    public Button falseButton;
    public Image flashImage;

    [Header("Flash Colors")]
    public Color correctColor = Color.green;
    public Color incorrectColor = Color.red;
    public float flashDuration = 0.5f;

    private bool isQuestionActive = false;
    private bool currentCorrectAnswer = false;
    private System.Action<bool> onAnswerCallback;

    void Start()
    {
        if (questionPanel != null)
            questionPanel.SetActive(false);
        
        if (flashImage != null)
        {
            Color c = flashImage.color;
            c.a = 0f;
            flashImage.color = c;
        }

        if (trueButton != null)
            trueButton.onClick.AddListener(() => AnswerQuestion(true));
        
        if (falseButton != null)
            falseButton.onClick.AddListener(() => AnswerQuestion(false));
    }

    public void ShowQuestion(string question, bool correctAnswer, System.Action<bool> callback)
    {
        if (isQuestionActive) return;

        isQuestionActive = true;
        currentCorrectAnswer = correctAnswer;
        onAnswerCallback = callback;

        Time.timeScale = 0f;

        if (questionPanel != null)
            questionPanel.SetActive(true);

        if (questionText != null)
            questionText.text = question;
    }

    void AnswerQuestion(bool playerAnswer)
    {
        if (!isQuestionActive) return;

        bool isCorrect = playerAnswer == currentCorrectAnswer;

        if (questionPanel != null)
            questionPanel.SetActive(false);

        StartCoroutine(FlashAndResume(isCorrect));

        onAnswerCallback?.Invoke(isCorrect);

        isQuestionActive = false;
    }

    IEnumerator FlashAndResume(bool correct)
    {
        if (flashImage != null)
        {
            Color flashColor = correct ? correctColor : incorrectColor;
            flashImage.color = flashColor;

            float elapsed = 0f;
            while (elapsed < flashDuration / 2f)
            {
                elapsed += Time.unscaledDeltaTime;
                float alpha = Mathf.Lerp(0f, 0.5f, elapsed / (flashDuration / 2f));
                Color c = flashImage.color;
                c.a = alpha;
                flashImage.color = c;
                yield return null;
            }

            elapsed = 0f;
            while (elapsed < flashDuration / 2f)
            {
                elapsed += Time.unscaledDeltaTime;
                float alpha = Mathf.Lerp(0.5f, 0f, elapsed / (flashDuration / 2f));
                Color c = flashImage.color;
                c.a = alpha;
                flashImage.color = c;
                yield return null;
            }

            Color finalColor = flashImage.color;
            finalColor.a = 0f;
            flashImage.color = finalColor;
        }

        Time.timeScale = 1f;
    }

    public bool IsQuestionActive()
    {
        return isQuestionActive;
    }
}