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
	// Axis-Aligned Bounding Box (AABB) 直方体の境界を計算
	glm::vec3 boxMin = w.pos - glm::vec3(w.width, w.height, w.depth) * 0.5f;
	glm::vec3 boxMax = w.pos + glm::vec3(w.width, w.height, w.depth) * 0.5f;

	glm::vec3 start = s.prevPos;		// 前フレーム位置
	glm::vec3 end = s.position;			// 現フレーム位置
	glm::vec3 d = end - start;			// 線分

	// 弾が手前の場合は,1以上の値となる、既に過ぎている場合は-の値となる
	// 計算で0-1の範囲となる場合は、衝突しているの考えられる
	float tmin = 0.0f;			// 衝突開始時刻
	float tmax = 1.0f;			// 衝突終了時刻

	for (int i = 0; i < 3; ++i)		// x,y,z軸毎に判定
	{
		if (std::abs(d[i]) < 1e-8f)		// 成分の絶対値が微小の場合
		{
			if (start[i] + s.sphereRadius < boxMin[i] || start[i] - s.sphereRadius > boxMax[i])
				return false; // 移動しない軸は、前フレーム位置が範囲外なら衝突なし
		}
		else
		{
			float invD = 1.0f / d[i];
			float t1 = (boxMin[i] - start[i] - s.sphereRadius) * invD;		// ボックス内に入った時刻
			float t2 = (boxMax[i] - start[i] + s.sphereRadius) * invD;		// ボックス外に出た時刻
			if (t1 > t2) std::swap(t1, t2);		// 進行方向によってスワップ
			tmin = std::max(tmin, t1);			// 各軸で最後にボックスを入った時刻
			tmax = std::min(tmax, t2);			// 各軸で最初にボックスを出た時刻
			if (tmin > tmax) return false; // 線分がボックスに入る前から出ている時、線分が箱と交差することはない
		}
	}

	// 衝突した場合の弾の位置調整
	glm::vec3 hitPos = start + d * tmin;

	// 最近接点 AABB の表面上で、球の中心に最も近い点 弾の中心:hitposとAABB表面は一致しない
	glm::vec3 closestPoint;
	for (int i = 0; i < 3; ++i)
	{
		if (hitPos[i] < boxMin[i]) closestPoint[i] = boxMin[i];		
		else if (hitPos[i] > boxMax[i]) closestPoint[i] = boxMax[i];
		else closestPoint[i] = hitPos[i];		// AABB範囲内
	}

	glm::vec3 diff = hitPos - closestPoint;		// 弾の中心->AABB表面の報告ベクトル
	float len = glm::length(diff);				// 正規化
	// 衝突法線
	glm::vec3 normal;
	if (len > 1e-8f)
	{
		// 普通の衝突 → diff から法線決定
		normal = diff / len;
	}
	else
	{
		// --- めり込み時の法線復元 ---------------------------

		// 球の中心とボックスの中心の差
		glm::vec3 delta = s.position - w.pos;

		// ボックスの半サイズ
		glm::vec3 half = glm::vec3(w.width, w.height, w.depth) * 0.5f;

		// 各軸のめり込み量（絶対距離）
		float dx = half.x - std::abs(delta.x);
		float dy = half.y - std::abs(delta.y);
		float dz = half.z - std::abs(delta.z);

		// 最もめり込んでいる軸を選択
		if (dx < dy && dx < dz)
			normal = glm::vec3((delta.x > 0 ? 1 : -1), 0, 0);
		else if (dy < dz)
			normal = glm::vec3(0, (delta.y > 0 ? 1 : -1), 0);
		else
			normal = glm::vec3(0, 0, (delta.z > 0 ? 1 : -1));
	}

	// -- - 衝突後の位置に修正-- -
	s.position = hitPos + normal * s.sphereRadius;

	// --- 完全反射 ---
	float vNormal = glm::dot(s.velocity, normal);
	if (vNormal < 0)
	{
		//s.velocity -= 2.0f * vNormal * normal; // 完全反射

		glm::vec3 v = s.velocity;

		// --- 速度を法線方向 / 接線方向に分解 ---
		float vNormal = glm::dot(v, normal);
		glm::vec3 vN = vNormal * normal;     // 法線方向
		glm::vec3 vT = v - vN;               // 接線方向

		// --- 反発係数（法線方向の反転） ---
		glm::vec3 vN_new = -vN * w.restitution;

		// --- 摩擦係数（接線方向の減衰） ---
		glm::vec3 vT_new = vT * (1.0f - w.friction);

		// --- 合成して新しい速度に ---
		s.velocity = vN_new + vT_new;

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

void drawSpheres(const glm::vec3& cameraPosition) {
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glColor3ub(255, 255, 0);				// 弾丸の色

	// 複数の弾の動的配列のループ
	for (size_t i = 0; i < spheres.size(); ++i) {
		Sphere& s = spheres[i];
		
		// --- 描画 ---
		float dist = glm::length(s.position - cameraPosition);
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
	s.position = cameraPos + right * GUN_MUZZLE_OFFSET.x + cameraUp * GUN_MUZZLE_OFFSET.y + cameraFront * GUN_MUZZLE_OFFSET.z;
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 加算から通常ブレンドへ
	glDepthMask(GL_TRUE);  // 深度を書き込む
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

// 弾の出る円筒の描画
void drawGunMuzzle(const Camera& camera, float radius) {
	glPushMatrix();

	// カメラの右・上ベクトル
	glm::vec3 camRight = glm::normalize(glm::cross(camera.front, camera.up));
	glm::vec3 camUp = camera.up;

	// 銃口の基準位置（右・上オフセットのみ）
	glm::vec3 basePos = camera.pos
		+ camRight * GUN_MUZZLE_OFFSET.x
		+ camUp * GUN_MUZZLE_OFFSET.y;

	glTranslatef(basePos.x, basePos.y, basePos.z);

	// gluCylinder は +Z方向に伸びるので、camera.front に向ける
	glm::vec3 defaultDir(0.0f, 0.0f, 1.0f); // gluCylinder のローカル Z+
	glm::vec3 targetDir = glm::normalize(camera.front);

	// 回転軸と角度を計算
	glm::vec3 axis = glm::cross(defaultDir, targetDir);
	float len = glm::length(axis);

	if (len > 1e-6f) {
		axis /= len; // 正規化
		float angle = glm::degrees(acos(glm::clamp(glm::dot(defaultDir, targetDir), -1.0f, 1.0f)));
		glRotatef(angle, axis.x, axis.y, axis.z);
	}

	// 円筒描画（長さ = offset.z）
	GLUquadric* quad = gluNewQuadric();
	gluQuadricNormals(quad, GLU_SMOOTH);
	gluCylinder(quad, radius, radius, GUN_MUZZLE_OFFSET.z-0.8f, 16, 1);
	gluDeleteQuadric(quad);

	glPopMatrix();
}

float getFireInterval() {
	return fireInterval;
}

void setFireInterval(float f) {
	fireInterval = f;
}
