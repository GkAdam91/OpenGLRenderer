#include "glwidget.h"
#include "mesh2.h"

#include <QDir>
#include <QtOpenGLExtensions/QOpenGLExtensions>
#include <qopenglext.h>
#include <QOpenGLExtraFunctions>
namespace viewer
{
	struct point
	{
		float x;
		float y;
		float z;
	};

	//ABuffer textures (pABufferUseTextures)
	GLuint abufferTexID = 0;
	GLuint abufferCounterTexID = 0;
	//ABuffer VBOs (pABufferUseTextures==0)
	GLuint abufferID = 0;
	GLuint abufferIdxID = 0;
	GLuint abufferMeshIdx = 0;


	//Mesh VBOs
	unsigned int frustrumVAO = 0;
	unsigned int frustrumVBO = 0;
	unsigned int frustrumEBO = 0;

	unsigned int lampVAO = 0;
	unsigned int lampVBO = 0;

	GLuint vertexBufferName = 0;
	GLuint vertexBufferModelPosID = 0;
	GLuint vertexBufferModelColorID = 0;
	GLuint vertexBufferModelNormalID = 0;
	GLuint vertexBufferModelUVID = 0;
	GLuint vertexBufferModelIndexID = 0;

	//Full screen quad vertices definition
	const GLfloat quadVArray[] = {
		-1.0f, -1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f
	};

	point v0 = { -0.5, -0.5, -0.5 };
	point v1 = { 0.5, -0.5, -0.5 };
	point v2 = { 0.5, 0.5, -0.5 };
	point v3 = { -0.5, 0.5, -0.5 };

	point v4 = { -0.5, -0.5, 0.5 };
	point v5 = { 0.5, -0.5, 0.5 };
	point v6 = { 0.5, 0.5, 0.5 };
	point v7 = { -0.5, 0.5, 0.5 };

	float scaleFrustrum[] = { 1.0, 1.0, 1.0 };

	float frustrumVertices[] = {

		v0.x, v0.y, v0.z,
		v1.x, v1.y, v1.z,
		v2.x, v2.y, v2.z,
		v2.x, v2.y, v2.z,
		v3.x, v3.y, v3.z,
		v0.x, v0.y, v0.z,

		v4.x, v4.y, v4.z,
		v5.x, v5.y, v5.z,
		v6.x, v6.y, v6.z,
		v6.x, v6.y, v6.z,
		v7.x, v7.y, v7.z,
		v4.x, v4.y, v4.z,

		v7.x, v7.y, v7.z,
		v3.x, v3.y, v3.z,
		v0.x, v0.y, v0.z,
		v0.x, v0.y, v0.z,
		v4.x, v4.y, v4.z,
		v7.x, v7.y, v7.z,

		v6.x, v6.y, v6.z,
		v2.x, v2.y, v2.z,
		v1.x, v1.y, v1.z,
		v1.x, v1.y, v1.z,
		v5.x, v5.y, v5.z,
		v6.x, v6.y, v6.z,

		v0.x, v0.y, v0.z,
		v1.x, v1.y, v1.z,
		v5.x, v5.y, v5.z,
		v5.x, v5.y, v5.z,
		v4.x, v4.y, v4.z,
		v0.x, v0.y, v0.z,

		v3.x, v3.y, v3.z,
		v2.x, v2.y, v2.z,
		v6.x, v6.y, v6.z,
		v6.x, v6.y, v6.z,
		v7.x, v7.y, v7.z,
		v3.x, v3.y, v3.z
	};
	GLuint64EXT abufferGPUAddress = 0;
	GLuint64EXT abufferCounterGPUAddress = 0;
	GLuint64EXT abufferMeshIndex = 0;

	GLWidget::GLWidget(QWidget* parent)
		: Scene(parent),
		m_vertexBuffer(QOpenGLBuffer::VertexBuffer),
		m_xRot(0),
		m_yRot(0),
		m_zRot(0),
		c_xRot(0),
		c_yRot(0),
		c_zRot(0),
		camRotX(0.0),
		camRotZ(0.0),
		camRotY(0.0),
		deg(0.0),
		m_shader(0),
		yVal(0),
		xVal(0),
		cyVal(0),
		cxVal(0),
		zoomVal(1.0),
		scaleVal(1.0),
		specVal(0.0),
		roughVal(0.0),
		showCube(false),
		diffTexture(true),
		pABufferUseTextures(0),
		normTexture(true),
		specTexture(true),
		isMetal(true),
		lightComboBoxIndex(0),
		isAlphaCorr(1),
		isGelly(1),
		isDispFragNum(0),
		isUseSort(1),
		showIndex(1)


	{
		HEIGHT = 1024;
		WIDTH = 1024;
		SCREEN_HEIGHT = HEIGHT;
		SCREEN_WIDTH = WIDTH;

	}

	GLWidget::~GLWidget()
	{
		cleanup();
	}



	void GLWidget::cleanup()
	{
		makeCurrent();
		m_vertexBuffer.destroy();
		if (m_shader) delete m_shader;
		m_shader = 0;
		doneCurrent();
	}

	QSize GLWidget::minimumSizeHint() const
	{
		return QSize(50, 50);
	}

	QSize GLWidget::sizeHint() const
	{
		return QSize(400, 400);
	}

	static void qNormalizeAngle(int &angle)
	{
		while (angle < 0)
			angle += 360 * 16;
		while (angle > 360 * 16)
			angle -= 360 * 16;
	}

	void GLWidget::setXRotation(int angle)
	{
		if (lightComboBoxIndex == 0){
			qNormalizeAngle(angle);
			if (angle != m_xRot) {
				m_xRot = angle;
				emit xRotationChanged(angle);
				update();
			}
		}
		else if (lightComboBoxIndex == 1){
			qNormalizeAngle(angle);
			if (angle != camRotX) {
				camRotX = angle;
				emit zRotationChanged(angle);
				update();
			}
		}
		else if (lightComboBoxIndex == 6){
			qNormalizeAngle(angle);
			if (angle != c_xRot) {
				c_xRot = angle;
				emit xRotationChanged(angle);
				update();
			}
		}
	}

	void GLWidget::setYRotation(int angle)
	{
		if (lightComboBoxIndex == 0){
			qNormalizeAngle(angle);
			if (angle != m_yRot) {
				m_yRot = angle;
				emit yRotationChanged(angle);
				update();
			}
		}
		else if (lightComboBoxIndex == 1){
			qNormalizeAngle(angle);
			if (angle != camRotY) {
				camRotY = angle;
				emit zRotationChanged(angle);
				update();
			}
		}
		else if (lightComboBoxIndex == 6){
			qNormalizeAngle(angle);
			if (angle != c_yRot) {
				c_yRot = angle;
				emit yRotationChanged(angle);
				update();
			}
		}
	}

	void GLWidget::setZRotation(int angle)
	{
		if (lightComboBoxIndex == 0){
			qNormalizeAngle(angle);
			if (angle != m_zRot) {
				m_zRot = angle;
				emit zRotationChanged(angle);
				update();
			}
		}
		else if (lightComboBoxIndex == 1){
			qNormalizeAngle(angle);
			if (angle != camRotZ) {
				camRotZ = angle;
				emit zRotationChanged(angle);
				update();
			}
		}
		else if (lightComboBoxIndex == 6){
			qNormalizeAngle(angle);
			if (angle != c_zRot) {
				c_zRot = angle;
				emit zRotationChanged(angle);
				update();
			}
		}
	}


	void GLWidget::translateUpDown(int direction, int dy)
	{
		if (lightComboBoxIndex == 0)
			yVal -= 0.1f*direction;
		else if (lightComboBoxIndex == 6){
			cyVal -= 0.1f*direction;
		}
		else{
			float ytemp = lightPositions[lightComboBoxIndex - 2].y();
			lightPositions[lightComboBoxIndex - 2].setY(ytemp - 0.2f*direction);
			qWarning() << ("yLightVal: " + to_string(lightPositions[lightComboBoxIndex - 2].y())).c_str();
		}
		update();
	}

	void GLWidget::translateLeftRight(int direction, int dx)
	{
		if (lightComboBoxIndex == 0)
			xVal -= 0.1f*direction;
		else if (lightComboBoxIndex == 1){
			
			setYRotation(camRotY + 8 * dx);
		}
		else if (lightComboBoxIndex == 6){
			cxVal -= 0.1f*direction;
		}
		else{
			float ytemp = lightPositions[lightComboBoxIndex - 2].x();
			lightPositions[lightComboBoxIndex - 2].setX(ytemp - 0.2f*direction);
			qWarning() << ("xLightVal: " + to_string(lightPositions[lightComboBoxIndex - 2].x())).c_str();
		}
		update();
	}



	void GLWidget::scaleSlider(int val)
	{
		scaleVal = (float)val / 10.0;
		qWarning() << ("scaleVal: " + to_string(scaleVal)).c_str();
		update();
	}

	void GLWidget::showIndexSlider(int val)
	{
		showIndex = val;
		qWarning() << ("showIndex: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::roughSlider(int val)
	{
		roughVal = val / 100.0;
		qWarning() << ("showIndex: " + to_string(roughVal)).c_str();
		update();
	}

	void GLWidget::specSlider(int val)
	{
		specVal = val / 100.0;
		qWarning() << ("showIndex: " + to_string(specVal)).c_str();
		update();
	}

	void GLWidget::diffTextureCheck(bool val)
	{
		diffTexture = val;
		qWarning() << ("diffuse: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::normTextureCheck(bool val)
	{
		normTexture = val;
		qWarning() << ("normal: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::specTextureCheck(bool val)
	{
		specTexture = val;
		qWarning() << ("specular: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::roughTextureCheck(bool val)
	{
		roughTexture = val;
		qWarning() << ("rough: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::isMetalCheck(bool val)
	{
		isMetal = val;
		qWarning() << ("isMetal: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::lightComboBox(int index)
	{
		qWarning() << ("index: " + to_string(index)).c_str();
		lightComboBoxIndex = index;
	}

	void GLWidget::isGellyCheck(bool val)
	{
		isGelly = val;
		qWarning() << ("isGelly: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::isUseSortCheck(bool val)
	{
		isUseSort = val;
		qWarning() << ("isUseSort: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::isAlphaCheck(bool val)
	{
		isAlphaCorr = val;
		qWarning() << ("isAlphaCorrect: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::isDispFragNumCheck(bool val)
	{
		isDispFragNum = val;
		qWarning() << ("isDispFragNum: " + to_string(val)).c_str();
		update();
	}

	void GLWidget::reload(){
		initShaders();
		initABuffer();
	}

	void GLWidget::printCheck(bool val){
		showCube = val;
	}

	void GLWidget::export3DModel(){
		QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
			"/home",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);
		ourModel.export3DModel(m_model2, m_model, dir);
	}


	void GLWidget::resetValues()
	{
		if (lightComboBoxIndex == 0){
		xVal = 0.0;
		yVal = 0.0;
		zoomVal = 1.0f;
		}
		else{
			lightPositions[lightComboBoxIndex - 2].setX(0.0);
			lightPositions[lightComboBoxIndex - 2].setY(0.0);
			lightPositions[lightComboBoxIndex - 2].setZ(0.0);
		}
		
	}


	void GLWidget::loadModel(){

		obj = new Objects();
		obj->sceneObjs = new SceneObjects();
		obj->face = new Face();
		obj->axes = new Axes(1.);
		obj->mesh = new Mesh();
		if (obj->imesh) delete obj->imesh;
		//ourModel = Model(QDir::currentPath().toStdString() + "/planes/boxes.obj", m_shader, m_vertexBuffer);

		//ourModel = Model(QDir::currentPath().toStdString()+"/planes/sphere.obj", m_shader, m_vertexBuffer);
		//          ourModel = Model(QDir::currentPath().toStdString()+"/bronze/bronzeSampled.obj", m_shader, m_vertexBuffer);
		//            ourModel = Model(QDir::currentPath().toStdString()+"/panel/layered_part_head.obj", m_shader, m_vertexBuffer);
		//            ourModel = Model(QDir::currentPath().toStdString()+"/Thesis Experiments/3 planes Hi/Hi.obj", m_shader, m_vertexBuffer);
		ourModel = Model(QDir::currentPath().toStdString() + "/merged/retopo.obj");
		// ourModel = Model(QDir::currentPath().toStdString()+"/Bronze_High_relief3.ply", m_shader, m_vertexBuffer);
		//ourModel = Model(QDir::currentPath().toStdString()+"/box/box2.obj", m_shader, m_vertexBuffer);
		//    ourModel = Model(QDir::currentPath().toStdString()+"/nano/nanosuit.obj", m_shader, m_vertexBuffer);
		//    ourModel = Model(QDir::currentPath().toStdString()+"/nymph/nymph_Layers2.obj", m_shader, m_vertexBuffer);
		//    ourModel = Model(QDir::currentPath().toStdString()+"/statue/dec.obj", m_shader, m_vertexBuffer);
		//    ourModel = Model(QDir::currentPath().toStdString()+"/bronzeMud/bronzeSampled.obj", m_shader, m_vertexBuffer);
		//    ourModel = Model(QDir::currentPath().toStdString()+"/gun/Cerberus_LP.FBX", m_shader, m_vertexBuffer);

		//    //    light0 = Model(QDir::currentPath().toStdString()+"/box/boxLight.obj", m_shader, m_vertexBuffer);
		//    //    light1 = Model(QDir::currentPath().toStdString()+"/box/boxLight.obj", m_shader, m_vertexBuffer);
		//    //    light2 = Model(QDir::currentPath().toStdString()+"/box/boxLight.obj", m_shader, m_vertexBuffer);
		//    //    light3 = Model(QDir::currentPath().toStdString()+"/box/boxLight.obj", m_shader, m_vertexBuffer);



	}

	void GLWidget::initializeGL()
	{
		lightPositions[0] = QVector3D(1.0f, 1.0f, 1.0f);
		lightPositions[1] = QVector3D(-10.0f, 10.0f, 0.0f);
		lightPositions[2] = QVector3D(10.0f, -10.0f, 10.0f);
		lightPositions[3] = QVector3D(10.0f, 10.0f, -10.00f);

		lightColors[0] = QVector3D(1.0f, 1.0f, 1.0f);
		lightColors[1] = QVector3D(1.0f, 1.0f, 1.0f);
		lightColors[2] = QVector3D(1.0f, 1.0f, 1.0f);
		lightColors[3] = QVector3D(1.0f, 1.0f, 1.0f);

		DisplayRender = 1;
		initializeOpenGLFunctions();
		QOpenGLExtension_EXT_shader_image_load_store *functions = new QOpenGLExtension_EXT_shader_image_load_store();
		functions->initializeOpenGLFunctions();
		QOpenGLFunctions_4_4_Core *f2 = new QOpenGLFunctions_4_4_Core();
		f2->initializeOpenGLFunctions();



		glClearColor(0.05f, 0.075f, 0.2f, 1.0f);


		initShaders();

		//Bind Quad
		f2->glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &vertexBufferName);
		f2->glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferName);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVArray), quadVArray, GL_STATIC_DRAW);
		f2->glBindVertexArray(0);

		
		//Bind Frustrum
		m_shaderSimple->bind();
		f2->glGenVertexArrays(1, &frustrumVAO);
		glGenBuffers(1, &frustrumVBO);
		f2->glBindVertexArray(frustrumVAO);
		glBindBuffer(GL_ARRAY_BUFFER, frustrumVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frustrumVertices), frustrumVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		f2->glBindVertexArray(0);
		m_shaderSimple->release();
		
		m_shaderDisp->bind();
		f2->glGenVertexArrays(1, &lampVAO);
		glGenBuffers(1, &lampVBO);
		f2->glBindVertexArray(lampVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frustrumVertices), frustrumVertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		f2->glBindVertexArray(0);
		m_shaderDisp->release();
		
		initABuffer();

		shaderBooleanUpdates();

		string pos = "lights[" + std::to_string(0) + "].position";
		for (int i = 0; i < 4; i++){
			m_shader->setUniformValue(("lightPositions[" + std::to_string(i) + "]").c_str(), lightPositions[i]);
			m_shader->setUniformValue(("lightColors[" + std::to_string(i) + "]").c_str(), lightColors[i]);
		}
		qWarning() << QDir::currentPath();

		loadModel();

		m_view.setToIdentity();
		m_view.translate(0, 0, -10);

		m_shader->release();

		//Disable backface culling to keep all fragments
		glEnable(GL_CULL_FACE);
		//Disable depth test
		glDisable(GL_DEPTH_TEST);
		//Disable stencil test
		glDisable(GL_STENCIL_TEST);
		//Disable blending
		glDisable(GL_BLEND);

		glDepthMask(GL_FALSE);
	}

	void GLWidget::paintGL()
	{
		QOpenGLFunctions_4_4_Core *f2 = new QOpenGLFunctions_4_4_Core();
		f2->initializeOpenGLFunctions();

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//    glEnable(GL_DEPTH_TEST);
		//    glEnable(GL_CULL_FACE);

		m_world.setToIdentity();
		//    m_world.translate(0.0f, 0.0f, zoomVal );
		m_world.translate(0.0f, 0.0f, scaleVal);
		m_world.rotate(180.0f - (camRotX / 16.0f), 1, 0, 0);
		m_world.rotate(camRotY / 16.0f, 0, 1, 0);
		m_world.rotate(camRotZ / 16.0f, 0, 0, 1);
		//    shaderBooleanUpdates();





		QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
		m_shader->bind();
		//Lights
		for (int i = 0; i < 4; i++){
			m_shader->setUniformValue(("lightPositions[" + std::to_string(i) + "]").c_str(), lightPositions[i]);
			//  m_shader->setUniformValue(("lightColors[" + std::to_string(i) + "]").c_str(), lightColors[i]);
		}
		projection();
		m_shader->setUniformValue(projectionID, scMat.projMat);
		m_shader->setUniformValue(view, m_view * m_world);
		m_shader->enableAttributeArray(0);
		m_shader->enableAttributeArray(1);
		m_shader->enableAttributeArray(2);

		m_model.setToIdentity();
		m_model.translate(xVal, yVal, 0.0f);
		m_model.scale(zoomVal);
		m_model.rotate(180.0f - (m_xRot / 16.0f), 1, 0, 0);
		m_model.rotate(m_yRot / 16.0f, 0, 1, 0);
		m_model.rotate(m_zRot / 16.0f, 0, 0, 1);
		m_shader->setUniformValue(model, m_model);


		m_shaderSimple->bind();
		//Bind Frustrum

		f2->glBindVertexArray(frustrumVAO);
		glBindBuffer(GL_ARRAY_BUFFER, frustrumVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frustrumVertices), frustrumVertices, GL_STATIC_DRAW);

		if (showCube){
			m_shaderSimple->bind();
			m_shaderSimple->setUniformValue("projection", scMat.projMat);
			m_shaderSimple->setUniformValue("view", m_view * m_world);
			m_shaderSimple->enableAttributeArray(0);
			m_model2.setToIdentity();

			m_model2.translate(xVal + cxVal, yVal + cyVal, 0.0f);
			m_model2.scale(scaleFrustrum[0], scaleFrustrum[1], scaleFrustrum[2]);
			m_model2.rotate(180.0f - (c_xRot / 16.0f), 1, 0, 0);
			m_model2.rotate(c_yRot / 16.0f, 0, 1, 0);
			m_model2.rotate(c_zRot / 16.0f, 0, 0, 1);

			m_shaderSimple->setUniformValue("model", m_model2);
		}

		
		DisplayRender = 1;


		displayClearABuffer_Basic();
		shaderBooleanUpdates();
		displayRenderABuffer_Basic();
		shaderBooleanUpdates();
		displayResolveABuffer_Basic();
		if (showCube){
			drawFrustrum();
		}
		//drawLights();
		DisplayRender = 1;
		update();
	}



	bool GLWidget::prepareShaderProgram(const QString& vertexShaderPath,
		const QString& fragmentShaderPath, const QString type)
	{
		QOpenGLShaderProgram *tmp = new QOpenGLShaderProgram;
		// First we load and compile the vertex shader…
		bool result = tmp->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderPath);
		if (!result)
			qWarning() << m_shader->log();

		// …now the fragment shader…
		result = tmp->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShaderPath);
		if (!result)
			qWarning() << m_shader->log();

		// …and finally we link them to resolve any references.
		result = tmp->link();
		if (!result)
			qWarning() << "Could not link shader program:" << tmp->log();

		if (type == "Clear"){
			m_shaderClear = tmp;
		}
		else if (type == "Disp"){
			m_shaderDisp = tmp;
		}
		else if (type == "Rend"){
			m_shader = tmp;
		}
		else if (type == "Simple"){
			m_shaderSimple = tmp;
		}
		else if (type == "light"){
			m_shaderLamp = tmp;
		}
		return result;
	}

	void GLWidget::shaderBooleanUpdates()
	{

		//    qWarning() <<  ("isAlphaCorrect: " + to_string(isAlphaCorr) + "     isDispFragNum: " + to_string(isDispFragNum) + "    isUseSort: " + to_string(isUseSort) +  "    isGelly: " + to_string(isGelly) + "     diffuse: " + to_string(pABufferUseTextures)).c_str();

		//        //Texture Booleans/*
		//
		if (DisplayRender == 1){
			m_shader->bind();
			if (diffTexture == true)
				m_shader->setUniformValue("diffBool", 1);
			else
				m_shader->setUniformValue("diffBool", 0);

			if (normTexture == true)
				m_shader->setUniformValue("normBool", 1);
			else
				m_shader->setUniformValue("normBool", 0);

			if (specTexture == true)
				m_shader->setUniformValue("specBool", 1);
			else
				m_shader->setUniformValue("specBool", 0);

			if (roughTexture == true)
				m_shader->setUniformValue("roughBool", 1);
			else
				m_shader->setUniformValue("roughBool", 0);

			if (isMetal == true)
				m_shader->setUniformValue("isMetallic", 1);
			else
				m_shader->setUniformValue("isMetallic", 0);
			DisplayRender = 0;
			m_shader->setUniformValue("SHOW_INDEX", showIndex);
			m_shader->setUniformValue("roughVal", roughVal);
			m_shader->setUniformValue("specVal", specVal);

		}
		else{

			m_shaderDisp->bind();
			m_shaderDisp->setUniformValue("SHOW_INDEX", showIndex);
			//    m_shaderDisp->release();
		}

	}

	void GLWidget::initShaders()
	{
		createClearShader();
		createRenderShader();
		createDispShader();
		createSimpleShader();
		createLampShader();
	}

	int GLWidget::createRenderShader()
	{
		m_shader = new QOpenGLShaderProgram;
		// Prepare a complete shader program…
		if (!prepareShaderProgram("./Data/pbrShader.vert", "./Data/pbrShader.frag", "Rend"))
			return 0;

		m_shader->bindAttributeLocation("aPos", 0);
		m_shader->bindAttributeLocation("aNormal", 1);
		m_shader->bindAttributeLocation("aTexCoords", 2);
		m_shader->link();

		m_shader->bind();
		projectionID = m_shader->uniformLocation("projection");
		model = m_shader->uniformLocation("model");
		view = m_shader->uniformLocation("view");
	}

	int GLWidget::createClearShader()
	{
		m_shaderClear = new QOpenGLShaderProgram;
		if (!prepareShaderProgram("./Data/passThrough.vert", "./Data/clearAbuffer.frag", "Clear"))
			return 0;
		m_shaderClear->bindAttributeLocation("aPos", 0);
		m_shaderClear->link();

		m_shaderClear->bind();
	}

	int GLWidget::createDispShader()
	{
		m_shaderDisp = new QOpenGLShaderProgram;
		if (!prepareShaderProgram("./Data/passThrough.vert", "./Data/dispABuffer.frag", "Disp"))
			return 0;
		m_shaderDisp->bindAttributeLocation("aPos", 0);
		m_shaderDisp->link();

		m_shaderDisp->bind();
	}

	int GLWidget::createSimpleShader(){
		m_shaderSimple = new QOpenGLShaderProgram;
		if (!prepareShaderProgram("./Data/simple.vert", "./Data/simple.frag", "Simple"))
			return 0;
		m_shaderSimple->bindAttributeLocation("aPos", 0);
		m_shaderSimple->link();

		m_shaderSimple->bind();
	}

	int GLWidget::createLampShader(){
		m_shaderLamp = new QOpenGLShaderProgram;
		if (!prepareShaderProgram("./Data/light.vert", "./Data/light.frag", "light"))
			return 0;
		m_shaderLamp->bindAttributeLocation("aPos", 0);
		m_shaderLamp->link();

		m_shaderLamp->bind();
	}


	void GLWidget::initABuffer(){
		QOpenGLExtension_EXT_shader_image_load_store *functions = new QOpenGLExtension_EXT_shader_image_load_store();
		QOpenGLExtension_NV_shader_buffer_load *functions2 = new QOpenGLExtension_NV_shader_buffer_load;
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		QOpenGLFunctions_4_4_Core *exf = new QOpenGLFunctions_4_4_Core;
		functions->initializeOpenGLFunctions();
		functions2->initializeOpenGLFunctions();
		f->initializeOpenGLFunctions();
		//    exf->initializeOpenGLFunctions();
		//Texture storage path
		if (pABufferUseTextures == 1){

			///ABuffer storage///
			if (abufferTexID == 0)
				glGenTextures(1, &abufferTexID);
			f->glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, abufferTexID);

			// Set filter
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			//Texture creation

			f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT * 32, 0, GL_RGBA, GL_FLOAT, 0);
			functions->glBindImageTextureEXT(0, abufferTexID, 0, true, 0, GL_READ_WRITE, GL_RGBA32F);

			//checkGLError ("AbufferTex");

			///ABuffer per-pixel counter///
			if (!abufferCounterTexID)
				glGenTextures(1, &abufferCounterTexID);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, abufferCounterTexID);

			// Set filter
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			//Texture creation
			//Uses GL_R32F instead of GL_R32I that is not working in R257.15
			f->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_FLOAT, 0);

			functions->glBindImageTextureEXT(1, abufferCounterTexID, 0, false, 0, GL_READ_WRITE, GL_R32UI);

			//checkGLError ("AbufferIdxTex");

		}
		else{	//Global memory storage path

			//Abuffer
			if (abufferID == 0)
				f->glGenBuffers(1, &abufferID);
			f->glBindBuffer(GL_ARRAY_BUFFER_ARB, abufferID);

			f->glBufferData(GL_ARRAY_BUFFER_ARB, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(float)* 4 * 32, NULL, GL_STATIC_DRAW);

			functions2->glGetBufferParameterui64vNV(GL_ARRAY_BUFFER_ARB, GL_BUFFER_GPU_ADDRESS_NV, &abufferGPUAddress);


			//AbufferIdx
			if (!abufferIdxID)
				f->glGenBuffers(1, &abufferIdxID);
			f->glBindBuffer(GL_ARRAY_BUFFER_ARB, abufferIdxID);

			glBufferData(GL_ARRAY_BUFFER_ARB, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(uint), NULL, GL_STATIC_DRAW);

			functions2->glGetBufferParameterui64vNV(GL_ARRAY_BUFFER_ARB, GL_BUFFER_GPU_ADDRESS_NV, &abufferCounterGPUAddress);

			if (!abufferMeshIdx)
				f->glGenBuffers(1, &abufferMeshIdx);
			f->glBindBuffer(GL_ARRAY_BUFFER_ARB, abufferMeshIdx);

			glBufferData(GL_ARRAY_BUFFER_ARB, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(float)* 32, NULL, GL_STATIC_DRAW);

			functions2->glGetBufferParameterui64vNV(GL_ARRAY_BUFFER_ARB, GL_BUFFER_GPU_ADDRESS_NV, &abufferMeshIndex);


		}


	}

	void GLWidget::drawQuad(QOpenGLShaderProgram *localShader) {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		f->initializeOpenGLFunctions();
		QOpenGLFunctions_4_4_Core *f2 = new QOpenGLFunctions_4_4_Core();
		f2->initializeOpenGLFunctions();
		int prog = localShader->programId();
		f->glUseProgram(prog);

		f2->glBindVertexArray(VAO);
		f->glEnableVertexAttribArray(glGetAttribLocation(prog, "aPos"));

		f->glBindBuffer(GL_ARRAY_BUFFER, vertexBufferName);

		f->glVertexAttribPointer(glGetAttribLocation(prog, "aPos"), 4, GL_FLOAT, GL_FALSE,
			sizeof(GLfloat)* 4, 0);
		f->glDrawArrays(GL_TRIANGLES, 0, 24);
		f2->glBindVertexArray(0);

	}

	void GLWidget::displayClearABuffer_Basic(){
		//Assign uniform parameters
		QOpenGLExtension_NV_shader_buffer_load *functions2 = new QOpenGLExtension_NV_shader_buffer_load;
		QOpenGLExtension_ARB_separate_shader_objects *functions = new QOpenGLExtension_ARB_separate_shader_objects();
		QOpenGLExtension_EXT_shader_image_load_store *functions3 = new QOpenGLExtension_EXT_shader_image_load_store();
		functions->initializeOpenGLFunctions();
		functions2->initializeOpenGLFunctions();
		functions3->initializeOpenGLFunctions();
		GLuint pID = m_shaderClear->programId();
		if (pABufferUseTextures == 1){

			functions->glProgramUniform1i(pID, glGetUniformLocation(pID, "abufferImg"), 0);
			functions->glProgramUniform1i(pID, glGetUniformLocation(pID, "abufferCounterImg"), 1);
		}
		else{
			functions2->glProgramUniformui64NV(pID, glGetUniformLocation(pID, "d_abuffer"), abufferGPUAddress);
			functions2->glProgramUniformui64NV(pID, glGetUniformLocation(pID, "d_abufferIdx"), abufferCounterGPUAddress);
			functions2->glProgramUniformui64NV(pID, glGetUniformLocation(pID, "d_abufferMeshIdx"), abufferMeshIndex);

		}

		//Render the full screen quad
		drawQuad(m_shaderClear);

		//Ensure that all global memory write are done before starting to render
		functions3->glMemoryBarrierEXT(GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV);
	}

	void GLWidget::drawModel(GLuint prog) {
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		f->initializeOpenGLFunctions();
		glUseProgram(prog);

		ourModel.Draw(m_shader);
	}

	void GLWidget::displayRenderABuffer_Basic(){
		QOpenGLExtension_ARB_separate_shader_objects *functions = new QOpenGLExtension_ARB_separate_shader_objects();
		QOpenGLExtension_NV_shader_buffer_load *functions2 = new QOpenGLExtension_NV_shader_buffer_load;
		functions->initializeOpenGLFunctions();
		functions2->initializeOpenGLFunctions();
		GLuint prog = m_shader->programId();
		//Assign uniform parameters
		if (pABufferUseTextures == 1){
			functions->glProgramUniform1i(prog, glGetUniformLocation(prog, "abufferImg"), 0);
			functions->glProgramUniform1i(prog, glGetUniformLocation(prog, "abufferCounterImg"), 1);

		}
		else{
			functions2->glProgramUniformui64NV(prog, glGetUniformLocation(prog, "d_abuffer"), abufferGPUAddress);
			functions2->glProgramUniformui64NV(prog, glGetUniformLocation(prog, "d_abufferIdx"), abufferCounterGPUAddress);
			functions2->glProgramUniformui64NV(prog, glGetUniformLocation(prog, "d_abufferMeshIdx"), abufferMeshIndex);
		}

		//Pass matrices to the shader

		functions->glProgramUniformMatrix4fv(prog, glGetUniformLocation(prog, "projection"), 1, GL_FALSE, scMat.projMat.constData());
		functions->glProgramUniformMatrix4fv(prog, glGetUniformLocation(prog, "model"), 1, GL_FALSE, m_model.constData());
		QMatrix4x4	modelViewMatrixIT = m_model.inverted().transposed();
		functions->glProgramUniformMatrix4fv(prog, glGetUniformLocation(prog, "modelIT"), 1, GL_FALSE, modelViewMatrixIT.constData());

		//Render the model

		drawModel(prog);
	}


	void GLWidget::displayResolveABuffer_Basic(){
		//Ensure that all global memory write are done before resolving
		QOpenGLExtension_EXT_shader_image_load_store *functions3 = new QOpenGLExtension_EXT_shader_image_load_store();
		QOpenGLExtension_ARB_separate_shader_objects *functions = new QOpenGLExtension_ARB_separate_shader_objects();
		QOpenGLExtension_NV_shader_buffer_load *functions2 = new QOpenGLExtension_NV_shader_buffer_load;
		functions->initializeOpenGLFunctions();
		functions2->initializeOpenGLFunctions();
		functions3->initializeOpenGLFunctions();
		functions3->glMemoryBarrierEXT(GL_SHADER_GLOBAL_ACCESS_BARRIER_BIT_NV);
		GLuint prog = m_shaderDisp->programId();
		//Assign uniform parameters
		if (pABufferUseTextures == 1){
			functions->glProgramUniform1i(prog, glGetUniformLocation(prog, "abufferImg"), 0);
			functions->glProgramUniform1i(prog, glGetUniformLocation(prog, "abufferCounterImg"), 1);
		}
		else{
			functions2->glProgramUniformui64NV(prog, glGetUniformLocation(prog, "d_abuffer"), abufferGPUAddress);
			functions2->glProgramUniformui64NV(prog, glGetUniformLocation(prog, "d_abufferIdx"), abufferCounterGPUAddress);
			functions2->glProgramUniformui64NV(prog, glGetUniformLocation(prog, "d_abufferMeshIdx"), abufferMeshIndex);
		}

		drawQuad(m_shaderDisp);
		drawLights();
	}

	void GLWidget::drawFrustrum(){
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

		QOpenGLFunctions_4_4_Core *f2 = new QOpenGLFunctions_4_4_Core();
		f2->initializeOpenGLFunctions();
		glDisable(GL_CULL_FACE);
		m_shaderSimple->bind();


		f2->glBindVertexArray(frustrumVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		f2->glBindVertexArray(0);
		glEnable(GL_CULL_FACE);
	}
	
	void GLWidget::drawLights(){
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

		QOpenGLFunctions_4_4_Core *f2 = new QOpenGLFunctions_4_4_Core();
		
		f2->initializeOpenGLFunctions();
		//Draw lights
		f2->glBindVertexArray(lampVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lampVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(frustrumVertices), frustrumVertices, GL_STATIC_DRAW);
		m_shaderLamp->bind();
		m_shaderLamp->setUniformValue("projection", scMat.projMat);
		m_shaderLamp->setUniformValue("view", m_view * m_world);
		m_shaderLamp->enableAttributeArray(0);
		
		for (int i = 0; i < 4; i++){
			m_modelLamp = QMatrix4x4();
			m_modelLamp.setToIdentity();
			m_modelLamp.translate(lightPositions[i].x(), lightPositions[i].y(), lightPositions[i].z());
			m_modelLamp.scale(0.5);
			m_shaderLamp->setUniformValue("model", m_modelLamp);
			glDisable(GL_CULL_FACE);
			f2->glBindVertexArray(lampVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glEnable(GL_CULL_FACE);
		}
	}

	void GLWidget::resizeGL(int w, int h)
	{
		WIDTH = w;
		HEIGHT = h;
		update();
	}



	void GLWidget::mousePressEvent(QMouseEvent *event)
	{
		m_lastPos = event->pos();

	}

	void GLWidget::mouseMoveEvent(QMouseEvent *event)
	{
		int dx = event->x() - m_lastPos.x();
		int dy = event->y() - m_lastPos.y();

		if (event->buttons() & Qt::LeftButton) {
			if (lightComboBoxIndex == 0){
				setXRotation(m_xRot + 8 * dy);
				setYRotation(m_yRot + 8 * dx);
			}
			else if (lightComboBoxIndex == 6){
				setXRotation(c_xRot + 8 * dy);
				setYRotation(c_yRot + 8 * dx);
			}
		}
		else if (event->buttons() & Qt::RightButton) {
			if (lightComboBoxIndex == 0){
				setXRotation(m_xRot + 8 * dy);
				setZRotation(m_zRot + 8 * dx);
			}
			else if (lightComboBoxIndex == 6){
				setXRotation(c_xRot + 8 * dy);
				setYRotation(c_zRot + 8 * dx);
			}
		}
		else if (event->buttons() & Qt::MiddleButton) {
			if (dx < -1)
				translateLeftRight(1, dx);
			else if (dx > 1)
				translateLeftRight(-1, dx);

			if (dy < -1)
				translateUpDown(-1, dy);
			else if (dy >1)
				translateUpDown(1, dy);
		}
		m_lastPos = event->pos();
	}

	void GLWidget::wheelEvent(QWheelEvent *event)
	{

		if (event->delta() > 0){
			if (lightComboBoxIndex == 0){

				zoomVal += 0.2f;
			}
			else if (lightComboBoxIndex == 1){
				scaleVal += 0.2f;
			}
			else if (lightComboBoxIndex == 6){
				scaleFrustrum[0] += 0.02f;
				scaleFrustrum[1] += 0.02f;
				scaleFrustrum[2] += 0.02f;
			}
			else{
				
				float ztemp = lightPositions[lightComboBoxIndex - 2].z();
				lightPositions[lightComboBoxIndex - 2].setZ(ztemp + 0.2f);
				
			}
		}
		else{
			if (lightComboBoxIndex == 0){

				if (zoomVal>0.2f)
					zoomVal -= 0.2f;
			}
			else if (lightComboBoxIndex == 1){
				scaleVal -= 0.2f;
			}
			else if (lightComboBoxIndex == 6){
				if (scaleFrustrum[0] > 0.0f)
					scaleFrustrum[0] -= 0.02f;
				if (scaleFrustrum[1] > 0.0f)
					scaleFrustrum[1] -= 0.02f;
				if (scaleFrustrum[2] > 0.0f)
					scaleFrustrum[2] -= 0.02f;
			}
			else{
				
				float ztemp = lightPositions[lightComboBoxIndex - 2].z();
				lightPositions[lightComboBoxIndex - 2].setZ(ztemp - 0.2f);
			}
		}
		update();
	}
}
