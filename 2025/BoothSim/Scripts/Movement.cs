using UnityEngine;
using UnityEngine.UI;

public class SpaceshipMovement : MonoBehaviour
{
    [Header("Movement Settings")]
    public float currentSpeed = 10f;
    public float strafeSpeed = 5f;
    public float strafeVisualBank = 25f;
    public float bankSmooth = 4f;

    [Header("Camera Settings")]
    public Camera playerCamera;
    public float pitchSpeed = 50f;
    public float pitchLimit = 45f;

    [Header("Boundaries")]
    public float leftBoundary = -3.2f;
    public float rightBoundary = 3.2f;
    public float topBoundary = 7f;
    public float bottomBoundary = 0.5f; // 👈 New bottom boundary

    [Header("UI Settings")]
    public Slider speedSlider;

    private float pitch = 0f;
    private float currentBank = 0f;

    void Start()
    {
        if (playerCamera == null)
            playerCamera = Camera.main;

        transform.rotation = Quaternion.Euler(0f, -90f, 0f);

        if (speedSlider != null)
        {
            speedSlider.value = currentSpeed;
            speedSlider.onValueChanged.AddListener(OnSpeedChanged);
        }
    }

    void OnSpeedChanged(float newSpeed)
    {
        currentSpeed = newSpeed;
    }

    void Update()
    {
        Vector3 pos = transform.position;

        // Forward movement
        pos += playerCamera.transform.forward * currentSpeed * Time.deltaTime;

        // Horizontal movement
        float horizontalInput = Input.GetAxis("Horizontal");
        pos += playerCamera.transform.right * horizontalInput * strafeSpeed * Time.deltaTime;

        // Clamp boundaries
        pos.z = Mathf.Clamp(pos.z, leftBoundary, rightBoundary);
        pos.y = Mathf.Clamp(pos.y, bottomBoundary, topBoundary); // 👈 Now clamps both top and bottom

        transform.position = pos;

        // Banking
        float targetBank = -horizontalInput * strafeVisualBank;
        currentBank = Mathf.Lerp(currentBank, targetBank, bankSmooth * Time.deltaTime);

        // Pitch (Up/Down look)
        float verticalInput = 0f;
        if (Input.GetKey(KeyCode.UpArrow)) verticalInput = 1f;
        if (Input.GetKey(KeyCode.DownArrow)) verticalInput = -1f;

        pitch -= verticalInput * pitchSpeed * Time.deltaTime;
        pitch = Mathf.Clamp(pitch, -pitchLimit, pitchLimit);

        transform.rotation = Quaternion.Euler(pitch, -90f, currentBank);

        // Camera follows
        if (playerCamera != null)
        {
            playerCamera.transform.position = transform.position;
            playerCamera.transform.rotation = transform.rotation;
        }
    }
}
