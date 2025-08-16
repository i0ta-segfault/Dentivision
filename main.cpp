#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "objectLoader.cpp"
#include "camera.cpp"
#include "shaders.cpp"
#include <fstream>
#include <curl/curl.h>
#include <cstring>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#define COLOR 45/255.0f 

#include "dependencies/imgui.h"
#include "dependencies/imgui_impl_glfw.h"
#include "dependencies/imgui_impl_opengl3.h"
#include "dependencies/imgui_demo.cpp"

const int screenHeight = 600, screenWidth = 800;
float fov = 45.f, lastFrame = .0f, deltaTime = .0f, movementSpeed = 2.f, lastX = screenWidth / 2.0f, lastY = screenHeight / 2.0f;
bool firstMouse = true, wireframeMode = false, rotating = false;
float makeTeethYellow = 0.213f, shrinkTeeth = 0.0f, dentalCalculus = 0.0f, gingivalRecession = 0.0f;

std::vector<float> vertices, colors;
std::vector<unsigned int> indices;
std::vector<int> type;

std::string userInput; // To store user input
std::string apiResponse;  // To store LLM response

glm::vec3 camPos = glm::vec3(0.0, 0.0, 7.0);
glm::vec3 camFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 camUp = glm::vec3(0.0, 1.0, 0.0);
glm::mat4 model = glm::mat4(1.0);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    ((std::string*)userp)->append((char*)contents, totalSize);
    return totalSize;
}

void sendToAPI(const std::string& input) {
    CURL* curl = curl_easy_init();
    if (curl) {
        CURLcode res;
        char* escapedInput = curl_escape(input.c_str(), static_cast<int>(input.length()));
        std::string apiKey = "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";
        std::string url = "https://api.openai.com/v1/chat/completions";

        std::string payload = R"({
            "model": "gpt-3.5-turbo",
            "messages": [
                {"role": "system", "content": "You are a helpful assistant that knows dental and medicine."},
                {"role": "user", "content": ")" + input + R"("}
            ]
        })";

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string headersString = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, headersString.c_str());

        // Configure cURL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &apiResponse);

        // Perform the request
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            apiResponse = "Error: " + std::string(curl_easy_strerror(res));
        } else {
            std::cout << "Raw API Response: " << apiResponse << std::endl;
            try {
                auto jsonResponse = json::parse(apiResponse);
                if (jsonResponse.contains("error")) {
                    apiResponse = "API Error: " + jsonResponse["error"]["message"].get<std::string>();
                } else if (jsonResponse.contains("choices") && !jsonResponse["choices"].empty()) {
                    apiResponse = jsonResponse["choices"][0]["message"]["content"];
                } else {
                    apiResponse = "Unexpected response format.";
                }
            } catch (const json::parse_error& e) {
                apiResponse = "JSON Parsing Error: " + std::string(e.what());
            }
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        apiResponse = "Failed to initialize cURL";
    }
}

void updateBufferWithString(const std::string& str, char* buffer, size_t bufferSize) {
    strncpy(buffer, str.c_str(), bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
}

void updateStringWithBuffer(std::string& str, const char* buffer) {
    str = buffer;
}

void setBuffers() {
    GLuint VAO, VBO, EBO, CBO, TypeBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &CBO);
    glGenBuffers(1, &TypeBO);

    glBindVertexArray(VAO);

    // Position buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Color buffer
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Type buffer
    glBindBuffer(GL_ARRAY_BUFFER, TypeBO);
    glBufferData(GL_ARRAY_BUFFER, type.size() * sizeof(int), type.data(), GL_STATIC_DRAW);
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(int), (void*)0);
    glEnableVertexAttribArray(2);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
    // Update camera aspect ratio
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera) {
        camera->screenWidth = static_cast<float>(width);
        camera->screenHeight = static_cast<float>(height);
    }
}

GLFWwindow* InitWindow(){
    if (!glfwInit()){
        std::cout << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Dentivision", NULL,NULL);

    if(!window) std::cout<<"Window was not created";
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }
    return window;
}

void pollKeyboardInputs(GLFWwindow* window, Camera& camera){
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.move(Camera::Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.move(Camera::Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.move(Camera::Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.move(Camera::Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.move(Camera::Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.move(Camera::Movement::DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void pollMouseInputs(GLFWwindow* window){
    GLFWcursor* crosshairCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        glfwSetCursor(window, crosshairCursor);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CROSSHAIR_CURSOR);
    }
    else {
        glfwSetCursor(window, nullptr);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (ImGui::IsAnyItemActive()) {
        // Skip rotation if interacting with ImGui elements
        firstMouse = true; // Reset firstMouse to avoid jumps after interaction
        return;
    }

    static Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xOffset = xpos - lastX;
        float yOffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
        lastX = xpos;
        lastY = ypos;

        //camera->rotate(xOffset, yOffset);

        // Apply rotation to the model matrix
        // Rotate around Y-axis for horizontal mouse movement (xOffset)
        float sensitivity = 0.3f;
        model = glm::rotate(model, glm::radians(xOffset * sensitivity), glm::vec3(0.0f, 1.0f, 0.0f));
        // Rotate around X-axis for vertical mouse movement (yOffset)
        model = glm::rotate(model, glm::radians(-yOffset * sensitivity), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    else{
        firstMouse = true;
    }
}

int main(){
    GLFWwindow* window = InitWindow();  
    if(!window) return -1;
    
    glViewport(0, 0, screenWidth, screenHeight);
    loadObj("2022-09-18_mouth_sketchfab.obj", vertices, indices, colors, type);

    setBuffers();

    std::string vertexShader = readFile("shader.vert"), fragmentShader = readFile("shader.frag");
    GLuint shader = CreateShader(vertexShader, fragmentShader);

    //create an object of the camera class
    Camera camera(camPos, camFront, camUp, fov, screenWidth, screenHeight, 0.1f, 100.0f);
    glfwSetWindowUserPointer(window, &camera); // Pass the camera to the mouse callback
    glfwSetCursorPosCallback(window, mouse_callback);

    const size_t INPUT_BUFFER_SIZE = 256; // Define a buffer size
    char inputBuffer[INPUT_BUFFER_SIZE] = ""; // Temporary buffer for input
    char responseBuffer[1024] = ""; // Temporary buffer for response (adjust size as needed)

    // Initialize buffer from std::string
    updateBufferWithString(userInput, inputBuffer, INPUT_BUFFER_SIZE);
    updateBufferWithString(apiResponse, responseBuffer, sizeof(responseBuffer));

    //-----imgui-----
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup ImGui backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
    //---------------

    glEnable(GL_DEPTH_TEST);
    //blending used to allow transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //-------------
    model = glm::scale(model, glm::vec3(15.0));
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float time = static_cast<float>(glfwGetTime()); 
        
        //Update
        pollKeyboardInputs(window, camera);
        pollMouseInputs(window);

        glm::mat4 view = camera.look();
        glm::mat4 projection = camera.project();
        if(rotating) 
            model = glm::rotate(model, (glm::radians(1.0f)), glm::vec3(0.0, 1.0, 0.0));

        if (wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Solid mode

        //Draw
        glClearColor(COLOR, COLOR, COLOR, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(90, 100), ImGuiCond_FirstUseEver);
        // GUI
        ImGui::Begin("Controls");
        ImGui::Checkbox("Wireframe Mode", &wireframeMode);
        ImGui::Checkbox("Rotating", &rotating);
        ImGui::SliderFloat("Bacterial Deposit over time", &makeTeethYellow, 0.0f, 1.0f);
        ImGui::SliderFloat("Dental Attrition", &shrinkTeeth, 0.0f, 1.0f);
        ImGui::SliderFloat("Dental Calculus", &dentalCalculus, 0.0f, 1.0f);
        ImGui::SliderFloat("Gingival Recession", &gingivalRecession, 0.0f, 0.1f);
        // Handle user input
        if (ImGui::InputText("Your Input", inputBuffer, INPUT_BUFFER_SIZE, ImGuiInputTextFlags_EnterReturnsTrue)) {
            updateStringWithBuffer(userInput, inputBuffer); // Update std::string
        }
        if (ImGui::Button("Send to LLM")) {
            apiResponse.clear(); // Clear the previous response
            sendToAPI(userInput); // Send input to the API
            updateBufferWithString(apiResponse, responseBuffer, sizeof(responseBuffer)); // Update buffer
        }

        // LLM Response
        ImGui::Text("LLM Response:");
        ImGui::InputTextMultiline("##Response", responseBuffer, sizeof(responseBuffer), ImVec2(-1, 200), ImGuiInputTextFlags_ReadOnly);
        ImGui::End();

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glUseProgram(shader);

        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(shader, "makeTeethYellow"), makeTeethYellow);
        glUniform1f(glGetUniformLocation(shader, "shrinkTeeth"), shrinkTeeth);
        glUniform1f(glGetUniformLocation(shader, "dentalCalculus"), dentalCalculus);
        glUniform1f(glGetUniformLocation(shader, "gingivalRecession"), gingivalRecession);
        glUniform1f(glGetUniformLocation(shader, "time"), time);
        glUniform3fv(glGetUniformLocation(shader, "camPos"), 1, glm::value_ptr(camPos));

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}