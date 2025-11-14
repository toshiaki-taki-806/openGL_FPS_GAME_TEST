#include "glut.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "weapon.h"
#include "camera.h"



std::vector<Sphere> spheres;
std::vector<DebugSphereInfo> debugSpheres;
LaserPointer laserPointer;
auto& prop = g_materialTable[static_cast<int>(MaterialType::RigidBody)];


// --- Analytical CCD ---
bool SweepSphereAABB(Sphere& s, const Wall& w) {
	glm::vec3 boxMin = w.pos - glm::vec3(w.width, w.height, w.depth) * 0.5f;
	glm::vec3 boxMax = w.pos + glm::vec3(w.width, w.height, w.depth) * 0.5f;

	glm::vec3 start = s.prevPos;
	glm::vec3 end = s.position;
	glm::vec3 d = end - start;

	float tmin = 0.0f;
	float tmax = 1.0f;

	for (int i = 0; i < 3; ++i)
	{
		if (std::abs(d[i]) < 1e-8f)
		{
			if (start[i] + s.sphereRadius < boxMin[i] || start[i] - s.sphereRadius > boxMax[i])
				return false; // 衝突なし
		}
		else
		{
			float invD = 1.0f / d[i];
			float t1 = (boxMin[i] - start[i] - s.sphereRadius) * invD;
			float t2 = (boxMax[i] - start[i] + s.sphereRadius) * invD;
			if (t1 > t2) std::swap(t1, t2);
			tmin = std::max(tmin, t1);
			tmax = std::min(tmax, t2);
			if (tmin > tmax) return false; // 衝突なし
		}
	}

	// 衝突した場合の位置
	glm::vec3 hitPos = start + d * tmin;

	// 最近接点（clampなしで手計算）
	glm::vec3 closestPoint;
	for (int i = 0; i < 3; ++i)
	{
		if (hitPos[i] < boxMin[i]) closestPoint[i] = boxMin[i];
		else if (hitPos[i] > boxMax[i]) closestPoint[i] = boxMax[i];
		else closestPoint[i] = hitPos[i];
	}

	glm::vec3 diff = hitPos - closestPoint;
	float len = glm::length(diff);
	glm::vec3 normal = (len > 1e-8f) ? diff / len : glm::vec3(0, 1, 0);

	// -- - 衝突後の位置に修正-- -
	s.position = hitPos + normal * s.sphereRadius;

	// --- 完全反射 ---
	float vNormal = glm::dot(s.velocity, normal);
	if (vNormal < 0)
	{
		s.velocity -= 2.0f * vNormal * normal; // 完全反射
	}

}


void updateSpheres() {
	float dt = static_cast<float>(PHYSICS_INTERVAL);	// 物理演算の間隔
	double currentTime = getTimeSec();					// 時刻を取得
	
	// 複数の弾の動的配列のループ
	for (auto it = spheres.begin(); it != spheres.end(); ) {
		double elapsed = currentTime - it->spawnTime;

		// --- 初回速度設定 ---
		if (!it->initialized) {
			glm::vec3 dir = glm::normalize(it->dir);
			it->velocity = dir * it->speed + it->inheritedVel;
			it->originPos = it->position;
			it->initialized = true;
		}
	
		//// デバッグ出力（初期値）
		{
		//	printf("x=%.3f y=%.3f, z=%.3f, , speed= %.3f, life=%.3f\n", it->position.x, it->position.y, it->position.z, glm::length(it->velocity), elapsed);
		//	printf("y=%.3f v=%.3f, life=%.3f, G=%.3f\n", it->position.y, it->velocity.y, elapsed, GRAVITY);
		//	printf("Spawn pos.y = %.3f (GROUND_Y=%.3f)\n", it->position.y, GROUND_Y);
		//	float v0 = glm::length(it->velocity);
		//	float area = 3.14159265358979323846f * it->sphereRadius * it->sphereRadius;
		//	float Fd0 = 0.5f * airDensity * dragCoefficient * area * v0 * v0;
		//	float a_drag0 = Fd0 / it->mass;
		//	printf("[spawn] v0=%.2f m/s mass=%.5f kg radius=%.6f m area=%.8f m2 Fd0=%.3f N a_drag0=%.1f m/s^2\n",
		//		v0, it->mass, it->sphereRadius, area, Fd0, a_drag0);
		}

		// --- 空気抵抗（速度二乗型） ---
		  // Cd を可変にしたければここで速度域により切り替える（亜音速/超音速など）
		float Cd = dragCoefficient;
		float area = 3.14159265358979323846f * it->sphereRadius * it->sphereRadius; // m^2 (投影面積)
		float v_len = glm::length(it->velocity);		// 速度のスカラー値

		glm::vec3 dragForce(0.0f);
		if (v_len > 1e-6f) {							//　速度が極小のときは処理をしない
			glm::vec3 v_dir = it->velocity / v_len;		// 単位ベクトル	長さ1
			// 抗力（力の向きは速度に逆向き）
			// Fd = 0.5 * rho * Cd * A * v^2  (方向は -v_dir)
			dragForce = -0.5f * airDensity * Cd * area * v_len * v_len * v_dir;		// fb=-1/2*ρ*Cd*A*v^2*方向ベクトル
		}

		// --- 重力 ---
		glm::vec3 gravity = glm::vec3(0.0f, -GRAVITY, 0.0f);

		// --- 加速度計算 ---
		glm::vec3 acceleration = gravity + (dragForce / it->mass);

		// --- 速度更新 ---
		it->velocity += acceleration * dt;

		// --- 位置更新 ---
		it->prevPos = it->position;		//　前回の位置を保持
		it->position += it->velocity * dt;
		glm::vec3 pos = it->position;

		// --- CCD 衝突判定 ---
		for (const Wall& w : g_walls) {
			if (SweepSphereAABB(*it, w)) {
				printf("衝突しました！球位置=(%.3f, %.3f, %.3f)\n", it->position.x, it->position.y, it->position.z);
			}
		}

		// --- 地面衝突判定 ---
		if (pos.y - it->sphereRadius <= GROUND_Y) {
			// デバッグ用
			/*float speed = glm::length(it->velocity);
			debugSpheres.push_back({ pos, speed, elapsed, currentTime + 4.0 });*/
			it->velocity.y = -it->velocity.y * prop.restitution;
			it->velocity.x *= (1.0f - prop.friction);
			it->velocity.z *= (1.0f - prop.friction);
			it->position.y = it->sphereRadius;
			//it = spheres.erase(it);
			//continue;
		}

		// 生存時間が過ぎたら消す　将来的には無くす？
		if (elapsed > it->duration) {
			// デバッグ用
			float speed = glm::length(it->velocity);
			debugSpheres.push_back({ pos, speed, elapsed, currentTime + 4.0 });
			it = spheres.erase(it);
			continue;
		}

		// --- 発射位置からの距離チェック ---
		if (glm::length(pos - it->originPos) > it->length) {
			// デバッグ用
			float speed = glm::length(it->velocity);
			debugSpheres.push_back({ pos, speed, elapsed, currentTime + 4.0 });
			it = spheres.erase(it);
			continue;
		}

		++it;
	}
}

void drawSpheres(const glm::vec3& cameraPos) {
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glColor3ub(255, 255, 0);				// 弾丸の色

	// 複数の弾の動的配列のループ
	for (size_t i = 0; i < spheres.size(); ++i) {
		Sphere& s = spheres[i];
		
		// --- 描画 ---
		float dist = glm::length(s.position - cameraPos);
		float scale = s.sphereRadius * s.renderScale / (dist * 0.05f + 1.0f);		// 距離による縮小

		glPushMatrix();
		glTranslatef(s.position.x, s.position.y, s.position.z);
		glutSolidSphere(scale, 12, 12);		//(GLdouble radius, GLint slices, GLint stacks);
		glPopMatrix();

	}
	glDisable(GL_BLEND);
}

// 発射関数
void fireSphere(const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp) {
	Sphere s;
	glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));
	// 発射位置をオフセット
	s.position = cameraPos + right * s.offset.x + cameraUp * s.offset.y + cameraFront * s.offset.z;
	// 方向
	s.dir = glm::normalize(cameraFront);
	
	// 出現時間→生存時間計算用
	s.spawnTime = getTimeSec();

	// ★ プレイヤーの慣性をコピー（走っていれば弾にも横速度が乗る）
	s.inheritedVel = playerVelocity;

	spheres.push_back(s); // 弾を追加
}

void drawLaserPointer() {
	if (!laserPointer.active) return;		// レーザーポインタオフのときは何もしない

	glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glColor3ub(255, 0, 0);					// レーザーの色

	// カメラ基準の発射位置
	glm::vec3 right = glm::normalize(glm::cross(camera.front, camera.up));		// カメラの右方向ベクトル
	// レーザーの発射位置をカメラからオフセット
	laserPointer.startPos = camera.pos
		+ right * laserPointer.offset.x
		+ camera.up * laserPointer.offset.y
		+ camera.front * laserPointer.offset.z;

	glm::vec3 dir = glm::normalize(camera.front);
	glm::vec3 end = laserPointer.startPos + dir * laserPointer.length;

	// 円柱の向きを計算
	glm::vec3 up(0, 1, 0);
	if (glm::length(glm::cross(up, dir)) < 0.001f) up = glm::vec3(1, 0, 0);
	glm::vec3 rightVec = glm::normalize(glm::cross(up, dir));
	glm::vec3 newUp = glm::cross(dir, rightVec);

	// モデル変換
	glPushMatrix();
	glTranslatef(laserPointer.startPos.x, laserPointer.startPos.y, laserPointer.startPos.z);

	// 回転行列で方向を調整
	GLfloat m[16] = {
		rightVec.x, rightVec.y, rightVec.z, 0,
		newUp.x,    newUp.y,    newUp.z,    0,
		dir.x,      dir.y,      dir.z,      0,
		0,          0,          0,          1
	};
	glMultMatrixf(m);

	GLUquadric* quad = gluNewQuadric();
	gluQuadricNormals(quad, GLU_SMOOTH);
	gluCylinder(quad, laserPointer.radius, laserPointer.radius, laserPointer.length, 12, 1);
	gluDeleteQuadric(quad);

	glPopMatrix();
	glPopAttrib();
}


float getFireInterval() {
	return fireInterval;
}

void setFireInterval(float f) {
	fireInterval = f;
}
