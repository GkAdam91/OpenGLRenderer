#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "../Viewer3D/scene.h"
#include <QOpenGLWidget>

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QMouseEvent>


#include "model.h"
#include "mesh2.h"


namespace viewer
{
	class GLWidget : public Scene
	{
		Q_OBJECT
	public:
		GLWidget(QWidget* parent = 0);
		~GLWidget();

		void diffTextureCheck(bool val);
		void normTextureCheck(bool val);
		void specTextureCheck(bool val);
		void roughTextureCheck(bool val);
		void isMetalCheck(bool val);
		void roughSlider(int val);
		void specSlider(int val);

	
		void translateUpDown(int direction, int dy);
		void translateLeftRight(int direction, int dx);
		
		void scaleSlider(int val);
		

		void resetValues();
		void lightComboBox(int index);
		void cleanup();
		void isDispFragNumCheck(bool val);
		void isAlphaCheck(bool val);
		void isUseSortCheck(bool val);
		void isGellyCheck(bool val);
		void showIndexSlider(int val);
		
		//export methods
		void printCheck(bool val);
		void export3DModel();
		void cancel3DExport();


		QSize minimumSizeHint() const Q_DECL_OVERRIDE;
		QSize sizeHint() const Q_DECL_OVERRIDE;	

		public slots:
		void setXRotation(int angle);
		void setYRotation(int angle);
		void setZRotation(int angle);

	signals:
		void xRotationChanged(int angle);
		void yRotationChanged(int angle);
		void zRotationChanged(int angle);

	protected:
		virtual void initializeGL();
		virtual void resizeGL(int w, int h);
		virtual void paintGL();
		void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void wheelEvent(QWheelEvent *event)Q_DECL_OVERRIDE;

		//virtual void keyPressEvent( QKeyEvent* e );

	private:
		bool prepareShaderProgram(const QString& vertexShaderPath,
			const QString& fragmentShaderPath,
			const QString type);
		void shaderBooleanUpdates();
		void initShaders();
		int createRenderShader();
		int createClearShader();
		int createDispShader();
		int createSimpleShader();
		int createLampShader();
		void initABuffer();
		void loadModel();
		
		Model ourModel;
		Model ourModel1;
		Model light0;
		Model light1;
		Model light2;
		Model light3;
		QOpenGLShaderProgram *m_shader;
		QOpenGLShaderProgram *m_shaderClear;
		QOpenGLShaderProgram *m_shaderDisp;
		QOpenGLShaderProgram *m_shaderSimple;
		QOpenGLShaderProgram *m_shaderLamp;

		QOpenGLBuffer m_vertexBuffer;
		QOpenGLBuffer *quad_vbo;
		QOpenGLVertexArrayObject m_vao;
		QOpenGLVertexArrayObject *m_vaoQuad;
		QMatrix4x4 m_view;
		QMatrix4x4 m_world;
		QMatrix4x4 m_model;
		QMatrix4x4 m_model2;
		QMatrix4x4 m_modelLamp;

		QMatrix4x4 m_proj;
		QPoint m_lastPos;
		int model;
		int view;
		int projectionID;
		int SCREEN_WIDTH;
		int SCREEN_HEIGHT;
		
		int m_xRot;
		int m_yRot;
		int m_zRot;

		int c_xRot;
		int c_yRot;
		int c_zRot;

		float yVal;
		float xVal;
		float cyVal;
		float cxVal;
		float camRotX;
		float camRotY;
		float camRotZ;

		float zoomVal;
		float scaleVal;
		float specVal;
		
		float deg;

		float roughVal;
		float aoVal;
		int showIndex;

		int lightComboBoxIndex;
		int pABufferUseTextures;
		unsigned int VAO;

		bool showCube;
		bool diffTexture;
		bool normTexture;
		bool specTexture;
		bool roughTexture;
		bool isMetal;
		bool isGelly;
		bool isUseSort;
		bool isAlphaCorr;
		bool isDispFragNum;
		QVector3D lightPositions[4];
		QVector3D lightColors[4];

		vector<float>		meshVertexPositionList;
		vector<float>		meshVertexNormalList;
		vector<float>		meshVertexUVList;
		vector<float>		meshVertexColorList;
		vector<uint>		meshTriangleList;

		vector<Mesh2> meshlist;
		vector<Mesh2> meshlist1;
		void displayClearABuffer_Basic(); 
		void drawQuad(QOpenGLShaderProgram *localShader);
		void displayRenderABuffer_Basic();
		void drawModel(GLuint prog);
		void displayResolveABuffer_Basic();
		void drawFrustrum();
		void drawLights();

		void reload();
		int DisplayRender;
	};
}
#endif // GLWIDGET_H
