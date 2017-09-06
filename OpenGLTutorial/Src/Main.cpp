/**
*	@file Main.cpp
*/

#include "GameEngine.h"
#include "C:/Users/tatsu/Desktop/OpenGLTutorial/OpenGLTutorial/Res/Audio/SampleSound_acf.h"
#include "C:/Users/tatsu/Desktop/OpenGLTutorial/OpenGLTutorial/Res/Audio/SampleCueSheet.h"

#include <glm/gtc/matrix_transform.hpp>


//�G���e�B�e�B�̏Փ˃O���[�vID
enum EntityGroupId
{
	EntityGroupId_Player,
	EntityGroupId_PlayerShot,
	EntityGroupId_Enemy,
	EntityGroupId_EnemyShot,
	EntityGroupId_Others,
};

//�Փˌ`�󃊃X�g
static const Entity::CollisionData collsionDataList[] =
{
	{ glm::vec3(-1.0f,-1.0f,-1.0f),glm::vec3(1.0f,1.0f,1.0f)},
	{ glm::vec3(-0.5f,-0.5f,-1.0f),glm::vec3(0.5f,0.5f,1.0f) },
	{ glm::vec3(-1.0f,-1.0f,-1.0f),glm::vec3(1.0f,1.0f,1.0f) },
	{ glm::vec3(-0.25f,-0.25f,-1.0f),glm::vec3(0.25f,0.25f,0.25f) },
};

///3D�x�N�^�[�^
struct Vector3
{
	float x, y, z;
};

///RGBA�J���[�^
struct Color
{
	float r, g, b, a;
};



///���_�V�F�[�_
static const char* vsCode =
"#version 410\n"
"layout(location = 0) in vec3 vPosition;"
"layout(location = 1) in vec4 vColor;"
"layout(location = 2) in vec2 vTexCoord;"
"layout(location = 0) out vec4 outColor;"
"layout(location = 1) out vec2 outTexCoord;"

"uniform mat4x4 matMVP;"	//new!
"void main(){"
" outColor = vColor;"
" outTexCoord = vTexCoord; "
" gl_Position = matMVP * vec4(vPosition,1.0);"	//new!
"}";

///�t���O�����g�V�F�[�_
static const char* fsCode =
"#version 410\n"
"layout(location = 0) in vec4 inColor;"
"layout(location = 1) in vec2 inTexCoord;"
"uniform sampler2D colorSampler;"
"out vec4 fragColor;"
"void main(){"
" fragColor = inColor * texture(colorSampler,inTexCoord);"
"}";













/**
*�G�̉~�Ղ̏�Ԃ��X�V����
*/
struct UpdateToroid
{
	//Delete
	//explicit UpdateToroid(Entity::BufferPtr buffer) : entityBuffer(buffer){}
	void operator() (Entity::Entity& entity,double delta)
	{
		//�͈͊O�ɏo����폜����
		const glm::vec3 pos = entity.Position();
		if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f)
		{
			GameEngine::Instance().RemoveEntity(&entity);
			return;
		}

		//�~�Ղ���]������
		float rot = glm::angle(entity.Rotation());
		rot += glm::radians(15.0f) * static_cast<float>(delta);
		if (rot > glm::pi<float>() * 2.0f)
		{
			rot -= glm::pi<float>() * 2.0f;
		}
		entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));

	}

	//Entity::BufferPtr entityBuffer;
};

/**
*	���@�̒e�̍X�V
*/
struct UpdatePlayerShot
{
	void operator() (Entity::Entity& entity, double delta)
	{
		const glm::vec3 pos = entity.Position();
		if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40)
		{
			entity.Destroy();
			return;
		}
	}
};

/**
*	�����̍X�V
*/
struct UpdateBlast
{
	void operator()(Entity::Entity& entity, double delta)
	{
		timer += delta;
		if (timer >= 0.5)
		{
			entity.Destroy();
			return;
		}
		//�ω���
		const float variation = static_cast<float>(timer * 4);
		//���X�Ɋg�傷��
		entity.Scale(glm::vec3(static_cast<float>(1 + variation)));

		//���Ԍo�߂ŐF�Ɠ����x��ω�������
		static const glm::vec4 color[] =
		{
			glm::vec4(1.0f,1.0f,0.75f,1),
			glm::vec4(1.0f,0.5f,0.1f,1),
			glm::vec4(0.25f,0.1f,0.1f,0),
		};

		const glm::vec4 col0 = color[static_cast<int>(variation)];
		const glm::vec4 col1 = color[static_cast<int>(variation) + 1];
		const glm::vec4 newColor = glm::mix(col0,col1,std::fmod(variation,1));
		entity.Color(newColor);
		
		//Y����]������
		glm::vec3 euler = glm::eulerAngles(entity.Rotation());
		euler.y += glm::radians(60.0f) * static_cast<float>(delta);
		entity.Rotation(glm::quat(euler));

	}
	double timer = 0;
};


/**
*	�����̍X�V
*/
struct UpdatePlayer
{
	void operator() (Entity::Entity& entity,double delta)
	{
		GameEngine& game = GameEngine::Instance();
		const GamePad gamepad = game.GetGamePad();
		glm::vec3 vec;
		float rotz = 0;
		if (gamepad.buttons & GamePad::DPAD_LEFT)
		{
			vec.x = 1;
			rotz = -glm::radians(30.0f);
		}
		else if (gamepad.buttons & GamePad::DPAD_RIGHT)
		{
			vec.x = -1;
			rotz = glm::radians(30.0f);
		}
		if (gamepad.buttons & GamePad::DPAD_UP)
		{
			vec.z = 1;
		}
		else if (gamepad.buttons & GamePad::DPAD_DOWN)
		{
			vec.z = -1;
		}
		if (vec.x || vec.z)
		{
			//����
			vec = glm::normalize(vec) * 10.0f;
		}
		entity.Velocity(vec);
		entity.Rotation(glm::quat(glm::vec3(0, 0, rotz)));
		glm::vec3 pos = entity.Position();
		pos = glm::min(glm::vec3(11, 100, 20), glm::max(pos, glm::vec3(-11, -100, 1)));
		entity.Position(pos);


		if (gamepad.buttons & GamePad::A)
		{
			shotInterval -= delta;
			if (shotInterval <= 0)
			{
				glm::vec3 pos = entity.Position();
				pos.x -= 0.3f;
				for (int i = 0;i < 2; ++i)
				{
					if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos,
						"NormalShot", "Res/Player.bmp", UpdatePlayerShot()))
					{
						p->Velocity(glm::vec3(0, 0, 80));
						p->Collision(collsionDataList[EntityGroupId_PlayerShot]);
					}
					pos.x += 0.6f;
				}
				shotInterval = 0.25;

				game.PlayAudio(0, CRI_SAMPLECUESHEET_PLAYERSHOT);
			}
		}
		else
		{
			shotInterval = 0;
		}


	}
private :
	double shotInterval = 0;
};


/**
*�Q�[���̏�Ԃ��X�V����
*
*@param	entityBuffer	�G�G���e�B�e�B�ǉ���̃G���e�B�e�B�o�b�t�@
*@param	meshBuffer		�G�G���e�B�e�B�̃��b�V�����Ǘ����Ă��郁�b�V���o�b�t�@
*@param	tex				�G�G���e�B�e�B�p�̃e�N�X�`��
*@param	prog			�G�G���e�B�e�B�p�̃V�F�[�_�[�v���O����
*/
struct Update
{
	void operator() (double delta)
	{
		GameEngine& game = GameEngine::Instance();

		if (!pPlayer)
		{
			pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2), "Aircraft", "Res/Player.bmp", UpdatePlayer());
			pPlayer->Collision(collsionDataList[EntityGroupId_Player]);
		}

		game.Camera({ glm::vec4(0,20,-8,1),glm::vec3(0,0,12),glm::vec3(0,0,1) });
		game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
		game.Light(0, { glm::vec4(40,100,10,1),glm::vec4(12000,12000,12000,1) });
		std::uniform_int_distribution<> distributerX(-12, 12);
		std::uniform_int_distribution<> distributerZ(40, 44);

		interval -= delta;
		if (interval <= 0)
		{
			const std::uniform_real_distribution<> rndInterval(1,1);
			const std::uniform_int_distribution<> rndAddingCount(1, 5);
			for (int i = rndAddingCount(game.Rand());i > 0; --i)
			{
				const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
				if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Toroid.bmp", UpdateToroid()))
				{
					p->Velocity(glm::vec3(pos.x < 0 ? 1.0f : -0.1f, 0, -10));
					p->Collision(collsionDataList[EntityGroupId_Enemy]);
				}
			}
			interval = rndInterval(game.Rand());
		}

		char str[16];
		snprintf(str, 16, "%08d", game.Score());
		game.FontScale(glm::vec2(2));
		game.FontColor(glm::vec4(1));
		game.AddString(glm::vec2(-0.2f, 0.9f), str);

	}
	double interval = 0;
	Entity::Entity* pPlayer = nullptr;
};


/**
*	���@�̒e�ƓG�̏Փˏ���
*/
void PlayerShotAndEntityCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs)
{
	GameEngine& game = GameEngine::Instance();
	if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(),
		"Blast", "Res/Toroid.bmp", UpdateBlast()))
	{
		const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
		p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
		game.Score(game.Score() + 100);
	}
	game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);
	lhs.Destroy();
	rhs.Destroy();
}


/**
*Uniform Block Object���쐬����
*
*@param	size Uniform Block�̃T�C�Y
*@param data Uniform Block�ɓ]������f�[�^�ւ̃|�C���^
*
*@return �쐬����UBO
*/
GLuint CreateUBO(GLsizeiptr size, const GLvoid* data = nullptr)
{
	GLuint ubo;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return ubo;
}


///�G���g���[�|�C���g
int main()
{
	GameEngine& game = GameEngine::Instance();
	if (!game.Init(800, 600, "OpenGL Tutorial"))
	{
		return 1;
	}
	if (!game.InitAudio("Res/Audio/SampleSound.acf", "Res/Audio/SampleCueSheet.acb", nullptr, CRI_SAMPLESOUND_ACF_DSPSETTING_DSPBUSSETTING_0))
	{
		return 1;
	}
	
	//TexturePtr tex = Texture::LoadFromFile("Res/Sparrow.bmp");
	game.LoadTextureFromFile("Res/Toroid.bmp");
	game.LoadTextureFromFile("Res/Player.bmp");
	//game.LoadTextureFromFile("Res/Sparrow.bmp");

	game.LoadMeshFromFile("Res/Toroid.fbx");
	game.LoadMeshFromFile("Res/Player.fbx");
	game.LoadMeshFromFile("Res/Blast.fbx");

	game.LoadFontFromFile("Res/font/UniNeue.fnt");

	game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy,
		&PlayerShotAndEntityCollisionHandler);

	game.UpdateFunc(Update());
	game.Run();
	
	return 0;
}