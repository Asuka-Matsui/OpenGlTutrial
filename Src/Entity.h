#pragma once
#include<GL/glew.h>
#include"Mesh.h"
#include"Texture.h"
#include"Shader.h"
#include"UniformBuffer.h"
#include"Uniform.h"
#include<glm/glm.hpp>
#include<glm/gtx/quaternion.hpp>
#include<memory>
#include<functional>
#include<vector>

namespace Entity {
	class Entity;
	class Buffer;
	typedef std::shared_ptr<Buffer> BufferPtr;	///< エンティティバッファポインタ型

	/// 衝突解決ハンドラ型
	typedef std::function<void(Entity&, Entity&)> CollisionHandlerType;

	static const int maxGroupID = 31;	///< グループ ID の最大値

	/**
	*	衝突判定形状
	*/
	struct CollisionData {
		glm::vec3 min;
		glm::vec3 max;
	};

	/**
	*	エンティティ
	*/
	class Entity {
		friend class Buffer;
	
	public:
		typedef std::function<void(Entity&, double)> UpdateFuncType;
		/// 状態更新関数型
//		typedef std::function<void(Entity&, void*, double, const glm::mat4&, const glm::mat4&)> UpdateFuncType;

		void Position(const glm::vec3& v) { position = v; }
		const glm::vec3& Position() const { return position; }
		void Rotation(const glm::quat& q) { rotation = q; }
		const glm::quat& Rotation() const { return rotation; }
		void Scale(const glm::vec3& v) { scale = v; }
		const glm::vec3& Scale() const { return scale; }
		void Color(const glm::vec4& v) { color = v; }
		const glm::vec4& Color() const { return color; }
		void Velocity(const glm::vec3& v) { velocity = v; }
		const glm::vec3& Velocity() const { return velocity; }
		void UpdateFunc(const UpdateFuncType& func) { updateFunc = func; }
		const UpdateFuncType& UpdateFunc() const { return updateFunc; }
		void Collision(const CollisionData& c) { colLocal = c; }
		const CollisionData& Collision() const { return colLocal; }

		glm::mat4 CalcModelMatrix() const;
		int GroupID() const { return groupID; }
		void Destroy();

	private:
		Entity() = default;
		~Entity() = default;
		Entity(const Entity&) = default;
		Entity& operator=(const Entity&) = default;

	private:
		int groupID = -1;						///< グループ ID
		Buffer* pBuffer = nullptr;				///< 生成元の Buffer クラスへのポインタ
		glm::vec3 position;						///< 座標
		glm::vec3 scale = glm::vec3(1, 1, 1);	///< 拡大率
		glm::vec4 color = glm::vec4(1, 1, 1, 1);///< 色
		glm::quat rotation;						///< 回転
		glm::vec3 velocity;						///< 速度
		UpdateFuncType updateFunc;				///< 状態更新関数
		CollisionData colLocal;					///< ローカル座標系の衝突形状
		CollisionData colWorld;					///< ワールド座標系の衝突形状
		Mesh::MeshPtr mesh;						///< エンティティを描画する時に使われるメッシュデータ
		TexturePtr texture;						///< エンティティを描画する時に使われるテクスチャ
		Shader::ProgramPtr program;				///< エンティティを描画する時に使われるシェーダ
		GLintptr uboOffset;						///< UBO のエンティティ用領域へのバイトオフセット
		bool isActive = false;					///< アクティブなエンティティなら true, 非アクティブなら false
	};

	/**
	*	エンティティバッファ
	*/
	class Buffer{
	public:
		static BufferPtr Create(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char*  name);

		Entity* AddEntity(int groupId, const glm::vec3& pos, const Mesh::MeshPtr& m, const TexturePtr& t, const Shader::ProgramPtr& p, Entity::UpdateFuncType func);
		void RemoveEntity(Entity* entity);
		void Update(double delta, const glm::mat4& matView, const glm::mat4& matProj);
		void Draw(const Mesh::BufferPtr& meshBuffer) const;

		void CollisionHandler(int gid0, int gid1, CollisionHandlerType handler);
		const CollisionHandlerType& collisionHandler(int gid0, int gid1) const;
		void ClearCollisionHandlerList();

	private:
		Buffer() = default;
		~Buffer() = default;
		Buffer(const Buffer&) = default;
		Buffer& operator=(const Buffer&) = default;

	private:
		///エンティティ用リンクリスト
		struct Link {
			void Insert(Link* e);
			void Remove();
			Link* prev = this;
			Link* next = this;
		};
		///リンク付エンティティ
		struct LinkEntity : public Link, public Entity {};
		///リンク付エンティティ配列の削除用関数オブジェクト
		struct EntityArrayDeleter { void operator()(LinkEntity* p) { delete[] p; } };

		std::unique_ptr<LinkEntity[], EntityArrayDeleter> buffer;	///< エンティティの配列
		size_t bufferSize;				///< エンティティの総数
		Link freeList;					///< 未使用のエンティティのリンクリスト
		Link activeList[maxGroupID + 1];///< 使用中のエンティティのリンクリスト
		UniformBufferPtr ubo;			///< エンティティ用 UBO
		Link* itrUpdate = nullptr;		///< Update と　RemoveEntity の相互作用に対応する為のイテレータ
		Link* itrUpdateRhs = nullptr;

		struct CollisionHandIerInfo{
			int groupID[2];
			CollisionHandlerType handler;
		};
		std::vector<CollisionHandIerInfo> collisionHandlerList;

		GLsizeiptr ubSizePerEntity;		///< 各エンティティが使える Uniform Buffer のバイト数
	};
}