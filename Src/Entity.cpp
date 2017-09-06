#include "Entity.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

/**
*	�G���e�B�e�B�Ɋւ���R�[�h���i�[���閼�O���
*/
namespace Entity {
	/**
	* VertexData��UBO�ɓ]������.
	*
	* @param entity
	* @param ubo
	* @param matViewProjection
	*/
	void UpdateUniformVertexData(Entity& entity, void* ubo, const glm::mat4& matVP)
	{
		Uniform::VertexData data;
		data.matModel = entity.CalcModelMatrix();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matVP * data.matModel;
		data.color = entity.Color();
		memcpy(ubo, &data, sizeof(data));
	}

	/**
	*	�ړ��E��]�E�g�k�s����擾����
	*
	*	@return TRS �s��
	*/
	glm::mat4 Entity::CalcModelMatrix() const {
		const glm::mat4 t = glm::translate(glm::mat4(), position);
		const glm::mat4 r = glm::mat4_cast(rotation);
		const glm::mat4 s = glm::scale(glm::mat4(), scale);

		return t * r * s;
	}

	/**
	*	�G���e�B�e�B��j������
	*
	*	���̊֐����ďo������́A�G���e�B�e�B�𑀍삵�Ă͂Ȃ�Ȃ�
	*/
	void Entity::Destroy() {
		if (pBuffer) {
			pBuffer->RemoveEntity(this);
		}
	}

	/**
	*	�������g�������N���X�g����؂藣��
	*
	*	�����͂ǂ��ɂ��ڑ�����Ă��Ȃ���ԂɂȂ�
	*/
	void Buffer::Link::Remove() {
		next->prev = prev;
		prev->next = next;
		prev = this;
		next = this;
	}

	/**
	*	�����N�I�u�W�F�N�g�������̎�O�ɒǉ�����
	*
	*	@param p	�ǉ����郊���N�I�u�W�F�N�g�ւ̃|�C���^
	*
	*	p ���������̃����N���X�g����؂藣���A�����̎�O�ɒǉ�����
	*/
	void Buffer::Link::Insert(Link* p) {
		p->Remove();
		p->prev = prev;
		p->next = this;
		prev->next = p;
		prev = p;
	}

	/**
	*	�G���e�B�e�B�o�b�t�@���쐬����
	*
	*	@param maxEntityCount	������G���e�B�e�B�̍ő吔
	*	@param ubSizePerEntity	�G���e�B�e�B���Ƃ� Uniform Buffer �̃o�C�g��
	*	@param bindingPoint		�G���e�B�e�B�p UBO �̃o�C���f�B���O�|�C���g
	*	@param ubName			�G���e�B�e�B�p Uniform Buffer �̖��O
	*
	*	@return �쐬�����G���e�B�e�B�o�b�t�@�ւ̃|�C���^
	*/
	BufferPtr Buffer::Create(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char*  ubName) {
		struct Impl : Buffer { Impl() {} ~Impl() {} };
		BufferPtr p = std::make_shared<Impl>();
		if (!p) {
			std::cerr << "WARNING in Entity::Buffer::Create: �o�b�t�@�̍쐬�Ɏ��s" << std::endl;
			return{};
		}
		p->ubo = UniformBuffer::Create(maxEntityCount * ubSizePerEntity, bindingPoint, ubName);
		p->buffer.reset(new LinkEntity[maxEntityCount]);
		if (!p->ubo || !p->buffer) {
			std::cerr << "WARNING in Entity::Buffer::Create: �o�b�t�@�̍쐬�Ɏ��s" << std::endl;
			return{};
		}
		p->bufferSize = maxEntityCount;
		p->ubSizePerEntity = ubSizePerEntity;
		GLintptr offset = 0;
		const LinkEntity* const end = &p->buffer[maxEntityCount];
		for (LinkEntity* itr = &p->buffer[0]; itr != end; ++itr) {
			itr->uboOffset = offset;
			itr->pBuffer = p.get();
			p->freeList.Insert(itr);
			offset += ubSizePerEntity;
		}
		p->collisionHandlerList.reserve(maxGroupID);
		return p;
	}

	/**
	*	�G���e�B�e�B��ǉ�����
	*
	*	@param groupId	�G���e�B�e�B�̃O���[�v ID
	*	@param position	�G���e�B�e�B�̍��W
	*	@param mesh		�G���e�B�e�B�̕\���Ɏg�p���郁�b�V��
	*	@param texture	�G���e�B�e�B�̕\���Ɏg�p����e�N�X�`��
	*	@param program	�G���e�B�e�B�̕\���Ɏg�p����V�F�[�_�v���O����
	*	@param func		�G���e�B�e�B�̏�Ԃ��X�V����֐�(�܂��͊֐��I�u�W�F�N�g)
	*
	*	@return �ǉ������G���e�B�e�B�ւ̃|�C���^
	*			����ȏ�G���e�B�e�B��ǉ��ł��Ȃ��ꍇ�� nullptr ���Ԃ����
	*			��]��g�嗦��ݒ肷��ꍇ�͂��̃|�C���^�o�R�ōs��
	*			���̃|�C���^���A�v���P�[�V�������ŕێ�����K�v�͂Ȃ�
	*/
	Entity* Buffer::AddEntity(int groupId, const glm::vec3& position, const Mesh::MeshPtr& mesh, const TexturePtr& texture, const Shader::ProgramPtr& program, Entity::UpdateFuncType func) {
		if (freeList.prev == freeList.next) {
			std::cout << "WARNING in Buffer::AddEntity;" "�󂫃G���e�B�e�B������܂���" << std::endl;
			return nullptr;
		}
		if (groupId < 0 || groupId > maxGroupID) {
			std::cout << "ERROR in Entity::Buffer::AddEntity: �͈͊O�̃O���[�v ID(" << groupId <<
				") ���n����܂���\n �O���[�v ID �́@0�`" << maxGroupID << "�łȂ���΂Ȃ�܂���" << std::endl;
			return nullptr;
		}

		LinkEntity* entity = static_cast<LinkEntity*>(freeList.prev);
		activeList[groupId].Insert(entity);
		entity->groupID = groupId;
		entity->position = position;
		entity->rotation = glm::quat();
		entity->scale = glm::vec3(1, 1, 1);
		entity->velocity = glm::vec3();
		entity->mesh = mesh;
		entity->texture = texture;
		entity->program = program;
		entity->updateFunc = func;
		entity->isActive = true;

		return entity;
	}

	/**
	*	�G���e�B�e�B���폜����
	*
	*	@param �폜����G���e�B�e�B�̃|�C���^
	*/
	void Buffer::RemoveEntity(Entity* entity) {
		if (!entity || !entity->isActive) {
			std::cout << "WARNING in Buffer::RemoveEntity;" "��A�N�e�B�u�ȃG���e�B�e�B���폜���悤�Ƃ��܂���" << std::endl;
			return;
		}
		LinkEntity* p = static_cast<LinkEntity*>(entity);
		if (p < &buffer[0] || p >= &buffer[bufferSize]) {
			std::cout << "WARNING in Buffer::RemoveEntity;" "�قȂ�o�b�t�@����G���e�B�e�B���폜���悤�Ƃ��܂���" << std::endl;
			return;
		}
		if (p == itrUpdate) {
			itrUpdate = p->prev;
		}
		if (p == itrUpdateRhs) {
			itrUpdateRhs = p->prev;
		}

		freeList.Insert(p);
		p->mesh.reset();
		p->texture.reset();
		p->program.reset();
		p->updateFunc = nullptr;
		p->isActive = false;
	}

	/**
	*	��`���m�̏Փ˔���
	*/
	bool HasCollision(const CollisionData& lhs, const CollisionData& rhs) {
		if (lhs.max.x < rhs.min.x || lhs.min.x > rhs.max.x) return false;
		if (lhs.max.y < rhs.min.y || lhs.min.y > rhs.max.y) return false;
		if (lhs.max.z < rhs.min.z || lhs.min.z > rhs.max.z) return false;
		return true;
	}
	
	/**
	*	�A�N�e�B�u�ȃG���e�B�e�B�̏�Ԃ��X�V����
	*
	*	@param delta	�O��̍X�V����̌o�ߎ���
	*	@param matView	View �s��
	*	@param matProj	Projection �s��
	*/
	void Buffer::Update(double delta, const glm::mat4& matView, const glm::mat4& matProj) {
		//���W�ƃ��[���h���W�n�̏Փˌ`����X�V����
		for (int groupId = 0; groupId <= maxGroupID; ++groupId) {
			for (Link* itr = activeList[groupId].next; itr != &activeList[groupId]; itr = itr->next) {
				LinkEntity& e = *static_cast<LinkEntity*>(itr);
				e.position += e.velocity * static_cast<float>(delta);
				e.colWorld.min = e.colLocal.min + e.position;
				e.colWorld.max = e.colLocal.max + e.position;
			}
		}

		//�Փ˔�������s����
		for (const auto& e : collisionHandlerList) {
			if (!e.handler) {
				continue;
			}
			Link* listL = &activeList[e.groupID[0]];
			Link* listR = &activeList[e.groupID[1]];
			for (itrUpdate = listL->next; itrUpdate != listL; itrUpdate = itrUpdate->next) {
				LinkEntity* entityL = static_cast<LinkEntity*>(itrUpdate);
				for (itrUpdateRhs = listR->next; itrUpdateRhs != listR; itrUpdateRhs = itrUpdateRhs->next) {
					LinkEntity* entityR = static_cast<LinkEntity*>(itrUpdateRhs);
					if (!HasCollision(entityL->colWorld, entityR->colWorld)) {
						continue;
					}
					e.handler(*entityL, *entityR);
					if (entityL != itrUpdate) {
						break;
					}
				}
			}
		}
		 //�e�G���e�B�e�B�̏�Ԃ��X�V����
		
		for (int groupId = 0; groupId <= maxGroupID; ++groupId) {
			for (itrUpdate = activeList[groupId].next; itrUpdate != &activeList[groupId]; itrUpdate = itrUpdate->next) {
				LinkEntity& e = *static_cast<LinkEntity*>(itrUpdate);
				e.position += e.velocity * static_cast<float>(delta);
				if (e.updateFunc) {
					e.updateFunc(e, delta);
				}
				e.colWorld.min = e.colLocal.min + e.position;
				e.colWorld.max = e.colLocal.max + e.position;
			}
		}
		itrUpdate = nullptr;
		itrUpdateRhs = nullptr;
//		ubo->UnmapBuffer();

		uint8_t* p = static_cast<uint8_t*>(ubo->MapBuffer());
		const glm::mat4 matVP = matProj * matView;
		for (int groupId = 0; groupId <= maxGroupID; ++groupId) {
			for (itrUpdate = activeList[groupId].next; itrUpdate != &activeList[groupId]; itrUpdate = itrUpdate->next) {
				LinkEntity& e = *static_cast<LinkEntity*>(itrUpdate);
				UpdateUniformVertexData(e, p + e.uboOffset, matVP);
			}
		}
		ubo->UnmapBuffer();
	}

	/**
	*	�A�N�e�B�u�ȃG���e�B�e�B��`�悷��
	*
	*	@param meshBuffer �`��Ɏg�p���郁�b�V���o�b�t�@�ւ̃|�C���^
	*/
	void Buffer::Draw(const Mesh::BufferPtr& meshBuffer) const {
		meshBuffer->BindVAO();
		for (int groupId = 0; groupId <= maxGroupID; ++groupId) {
			for (const Link* itr = activeList[groupId].next; itr != &activeList[groupId]; itr = itr->next) {
				const LinkEntity& e = *static_cast<const LinkEntity*>(itr);
				if (e.mesh && e.texture && e.program) {
					e.program->UseProgram();
					e.program->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, e.texture->Id());
					ubo->BufferRange(e.uboOffset, ubSizePerEntity);
					e.mesh->Draw(meshBuffer);
				}
			}
		}
	}

	/**
	*	�Փˉ����n���h����ݒ肷��
	*
	*	@param gid0		�ՓˑΏۂ̃O���[�v ID
	*	@param gid1		�ՓˑΏۂ̃O���[�v ID
	*	@param handler	�Փˉ����n���h��
	*
	*	�Փ˂��������Փˉ����n���h�����Ăяo����鎞�A��菬�����O���[�v ID �����G���e�B�e�B�����ɓn�����
	*	�����Ŏw�肵���O���[�v ID �̏����Ƃ͖��֌W�ł��邱�Ƃɒ���
	*	ex)
	*		CollisionHandler(10, 1, Func)
	*		�Ƃ����R�[�h�Ńn���h����o�^�����Ƃ���A�Փ˂����������
	*		Finc(�O���[�v ID = 1 �̃G���e�B�e�B�A�O���[�v ID = 10 �̃G���e�B�e�B)
	*		�̂悤�ɌĂяo�����
	*/
	void Buffer::CollisionHandler(int gid0, int gid1, CollisionHandlerType handler) {
		if (gid0 > gid1) {
			std::swap(gid0, gid1);
		}
		auto itr = std::find_if(collisionHandlerList.begin(), collisionHandlerList.end(), [&](const CollisionHandIerInfo& e) {
			return e.groupID[0] == gid0 && e.groupID[1] == gid1; });
		if (itr == collisionHandlerList.end()) {
			collisionHandlerList.push_back({ {gid0, gid1},handler });
		}else {
			itr->handler = handler;
		}
	}

	/**
	*	�Փˉ����n���h�����擾����
	*
	*	@param gid0		�ՓˑΏۂ̃O���[�v ID
	*	@param gid1		�ՓˑΏۂ̃O���[�v ID
	*
	*	@return	�Փˉ����n���h��
	*/
	const CollisionHandlerType& Buffer::collisionHandler(int gid0, int gid1) const {
		if (gid0 > gid1) {
			std::swap(gid0, gid1);
		}
		auto itr = std::find_if(collisionHandlerList.begin(), collisionHandlerList.end(), [&](const CollisionHandIerInfo& e) {
			return e.groupID[0] == gid0 && e.groupID[1] == gid1; });
		if (itr == collisionHandlerList.end()) {
			static const CollisionHandlerType dummy;
			return dummy;
		}
		return itr->handler;
	}

	/**
	*�S�Ă̏Փˉ����n���h�����폜����
	*/
	void Buffer::ClearCollisionHandlerList() {
		collisionHandlerList.clear();
	}


}
