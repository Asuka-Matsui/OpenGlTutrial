/**
* @file Main.cpp 
*/

#include"GameEngine.h"
#include<glm\gtc\matrix_transform.hpp>
#include<random>

using namespace Uniform;

//エンティティの衝突グループID
enum EntityGroupId {
	EntityGroupId_Player,
	EntityGroupId_PlayerShot,
	EntityGroupId_Enemy,
	EntityGroupId_EnemyShot,
	EntityGroupId_Others,
};

//衝突形状リスト
static const Entity::CollisionData collisionDataList[] = {
	{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
	{ glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
	{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
	{ glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
};

/// 3D ベクター型
struct Vector3 {
	float x, y, z;
};

/// RGBA カラー
struct Color {
	float r, g, b, a;
};

/**
*	敵の円盤の状態を更新する
*/
struct UpdateToroid {
		void operator()(Entity::Entity& entity, double delta) {
		//範囲外に出たら削除する
		const glm::vec3 pos = entity.Position();
		if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
			GameEngine::Instance().RemoveEntity(&entity);
			return;
		}

		//円盤を回転させる
		float rot = glm::angle(entity.Rotation());
		rot += glm::radians(15.0f) * static_cast<float>(delta);
		if (rot > glm::pi<float>() * 2.0f) {
			rot -= glm::pi<float>() * 2.0f;
		}
		entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));

		//頂点シェーダのパラメータを UBO にコピーする
/*		VertexData data;
		data.matModel = entity.CalcModelMatrix();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matProj * matView  * data.matModel;
		memcpy(ubo, &data, sizeof(VertexData));*/
	}
};

/**
*	自機弾の更新
*/
struct UpdatePlayerShot{
	void operator()(Entity::Entity& entity, double delta) {
		const glm::vec3 pos = entity.Position();
		if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40) {
			entity.Destroy();
			return;
		}
	}
};

/**
*	爆発の更新
*/
struct UpdateBlast {
	void operator()(Entity::Entity& entity, double delta) {
		timer += delta;
		if (timer >= 0.5) {
			entity.Destroy();
			return;
		}
		const float variation = static_cast<float>(timer * 4);		//変化量
		entity.Scale(glm::vec3(static_cast<float>(1 + variation)));	//徐々に拡大
		//時間経過で色と透明度を変化させる
		static const glm::vec4 color[] = {
			glm::vec4(1.0f, 1.0f, 0.75f, 1),
			glm::vec4(1.0f, 0.5f, 0.1f, 1),
			glm::vec4(0.25f, 0.5f, 0.1f, 0),
		};
		const glm::vec4 col0 = color[static_cast<int>(variation)];
		const glm::vec4 col1 = color[static_cast<int>(variation) + 1];
		const glm::vec4 newColor = glm::mix(col0, col1, std::fmod(variation, 1));
		entity.Color(newColor);
		// Y軸回転させる
		glm::vec3 euler = glm::eulerAngles(entity.Rotation());
		euler.y += glm::radians(60.0f) * static_cast<float>(delta);
		entity.Rotation(glm::quat(euler));
	}

	double timer = 0;
};

/**
*	自機の更新
*/
struct UpdatePlayer {
	void operator()(Entity::Entity& entity, double delta) {
		GameEngine& game = GameEngine::Instance();
		const GamePad gamepad = game.GetGamePad();
		glm::vec3 vec;
		float rotZ = 0;
		if (gamepad.buttons & GamePad::DPAD_LEFT) {
			vec.x = 1;
			rotZ = -glm::radians(30.0f);
		} else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
			vec.x = -1;
			rotZ = glm::radians(30.0f);
		}
		if (gamepad.buttons & GamePad::DPAD_UP) {
			vec.z = 1;
		}
		else if (gamepad.buttons & GamePad::DPAD_DOWN) {
			vec.z = -1;
		}
		if (vec.x || vec.z) {
			vec = glm::normalize(vec) * 2.0f;
		}
		entity.Velocity(vec);
		entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));
		glm::vec3 pos = entity.Position();
		pos = glm::min(glm::vec3(11, 100, 20), glm::max(pos, glm::vec3(-11, -100, 1)));
		entity.Position(pos);

		if (gamepad.buttons & GamePad::A) {
			shotInterval -= delta;
			if (shotInterval <= 0) {
				glm::vec3 pos = entity.Position();
				pos.x -= 0.3f;
				for (int i = 0; i < 2; ++i) {
					if (Entity::Entity* p = game.addEntity(EntityGroupId_PlayerShot, pos, "NormalShot", "Res/Player.bmp", UpdatePlayerShot())) {
						p->Velocity(glm::vec3(0, 0, 80));
						p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
					}
					pos.x += 0.6f;
				}
				shotInterval = 0.25;
			}
		} else {
			shotInterval = 0;
		}

		//頂点シェーダのパラメータを UBO にコピーする
	/*	VertexData data;
		data.matModel = entity.CalcModelMatrix();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matProj * matView  * data.matModel;
		memcpy(ubo, &data, sizeof(VertexData));*/
	}

private:
	double shotInterval = 0;
};

/**
* ゲームの状態を更新する
*
* @param entityBuffer	敵エンティティ追加先のエンティティバッファ
* @param meshBuffer		敵エンティティのメッシュを管理しているメッシュバッファ
* @param tex			敵エンティティ用のテクスチャ
* @param prog			敵エンティティ用のシェーダプログラム
*/
/*void Update(Entity::BufferPtr entityBuffer, Mesh::BufferPtr meshBuffer, TexturePtr tex, Shader::ProgramPtr prog) {
	static std::mt19937 rand(time(nullptr));
	static double interval = 0;

	interval -= 1.0 / 60.0;
	if (interval <= 0) {
		const std::uniform_real_distribution<float> posXRange(-15, 15);
		const glm::vec3 pos(posXRange(rand), 0, 40);
		const Mesh::MeshPtr& mesh = meshBuffer->GetMesh("Toroid");
		if (Entity::Entity* p = entityBuffer->AddEntity(pos, mesh, tex, prog, UpdateToroid(entityBuffer))) {
			p->Velocity(glm::vec3(pos.x < 0 ? 0.1f : -0.1f, 0, -1.0f));
		}
		const std::uniform_real_distribution<double> intervalRange(3.0, 6.0);
		interval = intervalRange(rand);
	}
}*/

struct Update {
	void operator()(double delta) {
		GameEngine& game = GameEngine::Instance();
		if (!pPlayer) {
			pPlayer = game.addEntity(EntityGroupId_Player, glm::vec3(0, 0, 2), "Aircraft", "Res/Player.bmp", UpdatePlayer());
			pPlayer->Collision(collisionDataList[EntityGroupId_Player]);
		}

		game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0,0,1) });
		game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
		game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000,12000,12000,1) });
		std::uniform_int_distribution<> distributerX(-12, 12);
		std::uniform_int_distribution<> distributerZ(40, 44);
		interval -= delta;
		if (interval <= 0) {
			const std::uniform_real_distribution<> rndInterval(0.5, 1.0);
			const std::uniform_int_distribution<> rndAddingCount(1, 5);
			for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
				const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
				if (Entity::Entity* p = game.addEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Toroid.bmp", UpdateToroid())) {
					p->Velocity(glm::vec3(pos.x < 0 ? 1.0f : -1.0f, 0, -10));
					p->Collision(collisionDataList[EntityGroupId_Enemy]);
				}
			}
			interval = rndInterval(game.Rand());
		}
	}

	double interval = 0;

	Entity::Entity* pPlayer = nullptr;
};

/**
*	自機の弾と敵の衝突処理
*/
void PlayerShotAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs) {
	GameEngine& game = GameEngine::Instance();
	if (Entity::Entity* p = game.addEntity(EntityGroupId_Others, rhs.Position(), "Blast", "Res/Toroid.bmp", UpdateBlast())) {
		const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
		p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
	}
	lhs.Destroy();
	rhs.Destroy();
}

void PlayerAndEnemyCollisionHandlerr(Entity::Entity& lhs, Entity::Entity& rhs) {
	GameEngine& game = GameEngine::Instance();
	if (Entity::Entity* p = game.addEntity(EntityGroupId_Others, rhs.Position(), "Blast", "Res/Toroid.bmp", UpdateBlast())) {
		const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
		p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
	}
	lhs.Destroy();
	rhs.Destroy();


}

/**
* Uniform Block Object を作成する
*
* @param size	Uniform Block のサイズ
* @param size	Uniform Block に転送するデータへのポインタ
*
* @return 作成した UBO
*/
GLuint CreateUBO(GLsizeiptr size, const GLvoid* data = nullptr) {
	GLuint ubo;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return ubo;
}

int main() {
	GameEngine& game = GameEngine::Instance();
	if (!game.Init(800, 600, "OpenGlTutrial")) {
		return -1;
	}
	game.LoadTextureFromFile("Res/Toroid.bmp");
	game.LoadTextureFromFile("Res/Player.bmp");
	game.LoadMeshFromFile("Res/Toroid.fbx");
	game.LoadMeshFromFile("Res/Player.fbx");
	game.LoadMeshFromFile("Res/Blast.fbx");
	game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &PlayerShotAndEnemyCollisionHandler);
//	game.CollisionHandler(EntityGroupId_Player, EntityGroupId_Enemy, &PlayerAndEnemyCollisionHandlerr);
	game.UpdateFunc(Update());
	game.Run();

	return 0;
}


///エントリーポイント
/*int main() {


	//三角形を書く
	//オブジェクトの作成
	const GLuint vbo = CreateVBO(sizeof(vertices), vertices);
	const GLuint ibo = CreateIBO(sizeof(indices), indices);
	const GLuint vao = CreateVAO(vbo, ibo);
	
	const UniformBufferPtr uboVertex = UniformBuffer::Create(
		sizeof(VertexData), 0, "VertexData");
	const UniformBufferPtr uboLight = UniformBuffer::Create(
		sizeof(LightData), 1, "LightData");
	const UniformBufferPtr uboPostEffect =
		UniformBuffer::Create(sizeof(PostEffectData), 2, "PostEffectData");

	const Shader::ProgramPtr progTutorial =
		Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");
	const Shader::ProgramPtr progColorFilter =
		Shader::Program::Create("Res/ColorFilter.vert", "Res/ColorFilter.frag");

	progColorFilter->UniformBlockBinding("PostEffectData", 2);

	if (!vbo || !ibo || !vao || !uboVertex || !uboLight || !progTutorial) {
		return 1;
	}
	progTutorial->UniformBlockBinding("VertexData", 0);
	progTutorial->UniformBlockBinding("LightData", 1);

	//glUseProgram 関数の呼び出しを変更
//	progTutorial->UseProgram();

	TexturePtr tex = Texture::LoadFpomFile("Res/Sample.bmp");
	TexturePtr texToroid = Texture::LoadFpomFile("Res/Toroid.bmp");
	if (!tex || !texToroid) {
		return 1;
	}

	Mesh::BufferPtr meshBuffer = Mesh::Buffer::Create(10 * 1024, 30 * 1024);
	meshBuffer->LoadMeshFromFile("Res/Toroid.fbx");

	Entity::BufferPtr entityBuffer = Entity::Buffer::Create(1024, sizeof(VertexData), 0, "VertexData");
	if (!entityBuffer) {
		return 1;
	}

	std::mt19937 rand(static_cast<unsigned int>(time(nullptr)));

	
	const OffscreenBufferPtr offscreen = OffscreenBuffer::Create(800, 600);


	//深度バッファ
	glEnable(GL_DEPTH_TEST);

	//テクスチャデータ
	static const uint32_t textureData[] = {
		0xffffffff, 0xffcccccc, 0xffffffff, 0xffcccccc, 0xffffffff,
		0xff888888, 0xffffffff, 0xff888888, 0xffffffff, 0xff888888,
		0xffffffff, 0xff444444, 0xffffffff, 0xff444444, 0xffffffff,
		0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
		0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
	};

	//メインループ
	while (!window.ShouldClose()) {
		Update(entityBuffer, meshBuffer, texToroid, progTutorial);
		glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFrameBuffer());

		glClearColor(0.1f,0.3f,0.5f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const glm::vec3 viewPos = glm::vec4(0, 20, -20, 1);

		//頂点データの描画
//		glUseProgram(progTutorial);
		progTutorial->UseProgram();

		//UBOにデータを転送する
//		glUseProgram(progTutorial);
		const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		VertexData vertexData;
		vertexData.matMVP = matProj * matView;
		uboVertex->BufferSubData(&vertexData);

		LightData lightData;
		lightData.ambientColor = glm::vec4(0.05f, 0.1f, 0.2f, 1);

		lightData.light[0].color = glm::vec4(12000, 12000, 12000, 1);
		lightData.light[0].position = glm::vec4(40, 100, 20, 1);
		uboLight->BufferSubData(&lightData);

		uboVertex->BufferSubData(&vertexData);

		progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, tex->Id());

#if 0
		glBindVertexArray(vao);
//		glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(vertices) / sizeof(vertices[0]));
		glDrawElements(
			GL_TRIANGLES, renderingParts[0].size,
			GL_UNSIGNED_INT, renderingParts[0].offset);
		
		progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, texToroid->Id());
		meshBuffer->BindVAO();
		meshBuffer->GetMesh("Toroid")->Draw(meshBuffer);
#endif
		entityBuffer->Update(1.0 / 60.0, matView, matProj);
		entityBuffer->Draw(meshBuffer);

#if 1
		glBindVertexArray(vao);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
/*		if (matMVPLoc >= 0) {
			glUniformMatrix4fv(matMVPLoc, 1, GL_FALSE, &glm::mat4()[0][0]);
			glBindTexture(GL_TEXTURE_2D, offscreen->GetTextur());
		}*/
/*		glBindVertexArray(vao);
		progColorFilter->UseProgram();

		PostEffectData postEffect;
		postEffect.matColor[0] = glm::vec4(0.393f, 0.349f, 0.272f, 0);
		postEffect.matColor[1] = glm::vec4(0.769f, 0.686f, 0.534f, 0);
		postEffect.matColor[2] = glm::vec4(0.189f, 0.168f, 0.131f, 0);
		postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
		uboPostEffect->BufferSubData(&postEffect);

		progColorFilter->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTextur());
		glDrawElements(
			GL_TRIANGLES, renderingParts[1].size,
			GL_UNSIGNED_INT, renderingParts[1].offset);
#endif
		window.SwapBuffers();
	}

//	glDeleteProgram(progTutorial);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	//終了処理
	glfwTerminate();

	return 0;
}*/


