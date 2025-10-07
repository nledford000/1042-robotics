using UnityEngine;
using TMPro;
using System.Collections.Generic;

public class ProceduralFloorGeneratorX : MonoBehaviour
{
    [Header("References")]
    public Transform player;
    public GameObject floorPrefab;
    public GameObject[] factoidPrefabs;
    public TMP_Text timerText;
    public TMP_Text pointsText;
    public FactoidQuestionSystem questionSystem;

    [Header("Settings")]
    public int tileSize = 10;
    public int tilesVisibleX = 20;
    public float updateInterval = 0.5f;
    public float baseFactoidSpawnChance = 0.1f;
    public float spawnIncreaseRate = 0.01f;
    public float factoidCollectDistance = 3f;
    public int pointsPerFactoid = 1;
    public int incorrectPenalty = 0;

    private float elapsedTime = 0f;
    private float factoidSpawnChance;
    private int lastTileX = 0;
    private Dictionary<int, GameObject> spawnedTiles = new Dictionary<int, GameObject>();
    private List<GameObject> spawnedFactoids = new List<GameObject>();
    private float updateTimer;
    private int playerPoints = 0;
    private bool timerPaused = false;

    void Start()
    {
        if (player == null)
            player = GameObject.FindGameObjectWithTag("Player").transform;

        factoidSpawnChance = baseFactoidSpawnChance;
        UpdateFloor();
        UpdatePointsUI();

        Debug.Log("ProceduralFloorGeneratorX started!");
        Debug.Log("Question System assigned: " + (questionSystem != null));
    }

    void Update()
    {
        if (!timerPaused)
        {
            elapsedTime += Time.deltaTime;
            if (timerText != null)
                timerText.text = $"Time: {elapsedTime:F1}s";

            factoidSpawnChance = Mathf.Clamp(baseFactoidSpawnChance + (elapsedTime * spawnIncreaseRate), 0f, 1f);
        }

        updateTimer += Time.deltaTime;
        if (updateTimer >= updateInterval)
        {
            updateTimer = 0f;
            UpdateFloor();
        }

        HandleFactoidCollection();
    }

    void UpdateFloor()
    {
        if (player == null || floorPrefab == null) return;

        int currentTileX = Mathf.FloorToInt(player.position.x / tileSize);

        for (int x = currentTileX - tilesVisibleX; x <= currentTileX + tilesVisibleX; x++)
        {
            if (!spawnedTiles.ContainsKey(x))
            {
                Vector3 tilePos = new Vector3(x * tileSize, 0, 0);
                GameObject tile = Instantiate(floorPrefab, tilePos, Quaternion.identity);
                spawnedTiles.Add(x, tile);

                if (factoidPrefabs.Length > 0 && Random.value < factoidSpawnChance)
                {
                    int index = Random.Range(0, factoidPrefabs.Length);
                    GameObject selectedFactoid = factoidPrefabs[index];

                    float randomZ = Random.Range(-5f, 5f);
                    float randomY = Random.Range(0f, 5f);
                    Vector3 factoidPos = tilePos + new Vector3(0, randomY, randomZ);

                    GameObject spawned = Instantiate(selectedFactoid, factoidPos, Quaternion.identity);
                    spawnedFactoids.Add(spawned);
                }
            }
        }

        List<int> toRemoveTiles = new List<int>();
        foreach (var tile in spawnedTiles)
        {
            if (Mathf.Abs(tile.Key - currentTileX) > tilesVisibleX + 5)
            {
                Destroy(tile.Value);
                toRemoveTiles.Add(tile.Key);
            }
        }
        foreach (int key in toRemoveTiles)
            spawnedTiles.Remove(key);

        lastTileX = currentTileX;
    }

    void HandleFactoidCollection()
    {
        if (player == null || spawnedFactoids.Count == 0) return;
        
        if (questionSystem != null && questionSystem.IsQuestionActive()) return;

        for (int i = spawnedFactoids.Count - 1; i >= 0; i--)
        {
            GameObject factoid = spawnedFactoids[i];
            if (factoid == null)
            {
                spawnedFactoids.RemoveAt(i);
                continue;
            }

            float distance = Vector3.Distance(player.position, factoid.transform.position);

            if (distance <= factoidCollectDistance && Input.GetKeyDown(KeyCode.Space))
            {
                Debug.Log("Space pressed! Distance: " + distance);

                Factoid factoidData = factoid.GetComponent<Factoid>();
                
                if (factoidData != null && questionSystem != null)
                {
                    Debug.Log("Showing question!");
                    timerPaused = true;
                    spawnedFactoids.RemoveAt(i);
                    GameObject collectedFactoid = factoid;
                    
                    questionSystem.ShowQuestion(
                        factoidData.question,
                        factoidData.correctAnswer,
                        (isCorrect) => OnQuestionAnswered(isCorrect, collectedFactoid)
                    );
                }
                else
                {
                    Debug.LogWarning("FactoidData not found or QuestionSystem is null!");
                    playerPoints += pointsPerFactoid;
                    UpdatePointsUI();
                    Destroy(factoid);
                    spawnedFactoids.RemoveAt(i);
                }
            }
        }
    }

    void OnQuestionAnswered(bool correct, GameObject factoid)
    {
        timerPaused = false;
        
        if (correct)
        {
            playerPoints += pointsPerFactoid;
            Debug.Log("Correct! Points: " + playerPoints);
        }
        else
        {
            playerPoints -= incorrectPenalty;
            playerPoints = Mathf.Max(0, playerPoints);
            Debug.Log("Incorrect! Points: " + playerPoints);
        }
        
        UpdatePointsUI();
        Destroy(factoid);
    }

    void UpdatePointsUI()
    {
        if (pointsText != null)
            pointsText.text = $"Points: {playerPoints}";
    }
}