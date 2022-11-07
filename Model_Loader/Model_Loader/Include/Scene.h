#ifndef SCENE_H
#define SCENE_H

// Forward decleration
struct GLFWwindow;
class Camera;
class Shader;
class Model;

class Scene
{
public:
	static Scene* GetInstance();

	// initial setup function
	bool Initialise();
	// processes to be done each frame
	bool Update();
	// rendering each frame
	void Render();
	// closes everything down when the application is closed
	void Deinitialise();

private:
	// constructors
	Scene();
	Scene(const Scene&);
	Scene& operator=(const Scene&);

	// adjusts the window
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	// mouse pos
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	// scroll 
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	// random values
	int RandomIntBewtweenRange(int a_iLowerRange, int a_iUpperRange);
	float RandomFloatBetweenRange(float a_fLowerRange, float a_fUpperRange);

	// updates the values for the weights of each force on the boids
	void UpdateBoidWeights();
	// modifies boid number
	void UpdateBoidNumber();
	
	GLFWwindow* m_window;
	Camera* m_camera;
	Shader* m_shader;
	Model* m_model;

	float m_lastX;
	float m_lastY;
	bool m_firstMouse;

	float m_deltaTime;
	float m_lastFrame;

	float m_boundingBoxSize;

	static Scene* s_pSceneInstance;

	float wanderWeight = 0.5f;
	float allignmentWeight = 0.5f;
	float cohesionWeight = 0.5f;
	float separationWeight = 0.5f;

	float minWeight = 0.0f;
	float maxWeight = 1.0f;

	int maxBoids = 700;
	int minBoids = 1;
	int numBoids = 10;

	int m_iSizeOfGroup = 0;
	int m_iCurrentGroupNum = 0;

protected:
	// functions of the ImGui content
	void showFrameData(bool a_bShowFrameData);
	void changeBehaviourWeights(bool a_bShowBehaviour);
};

#endif //!SCENE_H

