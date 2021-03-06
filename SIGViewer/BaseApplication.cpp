/*
-----------------------------------------------------------------------------
Filename: BaseApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "resource.h"
#include "BaseApplication.h"
#include "SIGVerse.h"
//#include "OgreOculus/OgreOculus.h"
//-------------------------------------------------------------------------------------
BaseApplication::BaseApplication(void)
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
    mSetupSuccessful(false),
    mBackGroundColor(0.5f,0.5f,0.7f,1.0f),
    oculusMode(false),
    openvrMode(false),
    fullscreenMode(false),
    hmdCameraFlag(false)
{
}

//-------------------------------------------------------------------------------------
BaseApplication::~BaseApplication(void)
{
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);

    if (mRoot != NULL) {
        //delete mRoot;
        //mRoot = NULL;
    }
}

//-------------------------------------------------------------------------------------
bool BaseApplication::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg

    Ogre::RenderSystem *rs = mRoot->getRenderSystemByName("OpenGL Rendering Subsystem");
    Ogre::RenderSystemList renderers = mRoot->getAvailableRenderers();
    Ogre::LogManager::getSingleton().logMessage(Ogre::LML_NORMAL, "Available Renderers:");
    for (size_t i = 0; i < renderers.size(); i++)
    {
        Ogre::LogManager::getSingleton().logMessage(Ogre::LML_NORMAL, renderers[i]->getName());
    }

    mRoot->setRenderSystem(rs);
    
    if(fullscreenMode){
        rs->setConfigOption("Full Screen", "Yes");
        rs->setConfigOption("Video Mode", "1280 x 720 @ 32-bit colour");
    }
    else{
        rs->setConfigOption("Full Screen", "No");
        rs->setConfigOption("Video Mode", "1024 x 768 @ 32-bit colour");
    }
    //rs->setConfigOption("Multi device memory hint", "Auto hardware buffers management");
    mWindow = mRoot->initialise(true, "SIGViewer");
    
    mWindow->setDeactivateOnFocusChange(false);

    // Icon setting.

#ifdef WIN32
    HWND hwnd;
    mWindow->getCustomAttribute("WINDOW", &hwnd);
    HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
    SetClassLong (hwnd, GCL_HICON, (LONG)LoadIcon (hInst, MAKEINTRESOURCE (IDI_ICON1)));
 #endif

    return true;
}
//-------------------------------------------------------------------------------------
void BaseApplication::chooseSceneManager(void)
{
    //create a scene manager that is meant for handling outdoor scenes
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_EXTERIOR_CLOSE);

    //// Get the SceneManager, in this case a generic one
    //mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}
//-------------------------------------------------------------------------------------
void BaseApplication::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    //mCamera->setPosition(Ogre::Vector3(103.4f, 34.3f, 65.9f));

    // Look back along -Z
    //mCamera->lookAt(Ogre::Vector3(-0.5f, -0.2f, -0.8f));
    mCamera->setNearClipDistance(5);

//	mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}
//-------------------------------------------------------------------------------------
void BaseApplication::createFrameListener(void)
{
    mLMouseDown    = false;
    mRMouseDown    = false;
    mShift         = false;
    mCtrl          = false;
    mWindowResized = false;

    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
 
    mWindow->getCustomAttribute("WINDOW", &windowHnd);

    windowHndStr << windowHnd;

    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem( pl );
 
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setVisible(false);

    //Set initial mouse clipping size
    windowResized(mWindow);
 
    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    mRoot->addFrameListener(this);

    Ogre::LogManager::getSingletonPtr()->logMessage("*** OIS Initialization Complete ***");
}
//-------------------------------------------------------------------------------------
void BaseApplication::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void BaseApplication::createViewports(void)
{
    // Create one viewport, entire window
    if (oculusMode)
        mViewPort = mWindow->addViewport(oculus.m_cameras[0]);
    else if (openvrMode)
        mViewPort = mWindow->addViewport(openvr.m_cameras[0]);
    else
        mViewPort= mWindow->addViewport(mCamera);
    mViewPort->setBackgroundColour(mBackGroundColor);

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(Ogre::Real(mViewPort->getActualWidth()) / Ogre::Real(mViewPort->getActualHeight()));

}
//-------------------------------------------------------------------------------------
void BaseApplication::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void BaseApplication::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void BaseApplication::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    if (oculusMode)
    {
        Ogre::LogManager::getSingleton().logMessage("Rendering Start (OculusMode == TRUE)");
        while (!this->mWindow->isClosed())
        {
            oculus.Update();
            Ogre::WindowEventUtilities::messagePump();
        }
    }
    else if (openvrMode)
    {
        Ogre::LogManager::getSingleton().logMessage("Render Start (OpenVRMode == TRUE)");
        while (!this->mWindow->isClosed())
        {
            openvr.update();
            Ogre::WindowEventUtilities::messagePump();
        }
    }
    else
    {
        Ogre::LogManager::getSingleton().logMessage("Rendering Start (OculusMode == FALSE)");
        mRoot->startRendering();
    }
    

    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool BaseApplication::setup(void)
{
    char dir[MAX_STRING_NUM];
    GetCurrentDirectory(MAX_STRING_NUM, dir);
    std::string inipath = std::string(dir) + "/SIGVerse.ini";
    TCHAR SettingPath[256];
    sprintf_s(SettingPath, 128, inipath.c_str());
    TCHAR pathText[256];
    GetPrivateProfileString("MODE","HMD_MODE",'\0', pathText, 1024, SettingPath);
    if(strcmp(pathText,"oculus") == 0)  oculusMode = true;
    if (strcmp(pathText, "openvr") == 0)  openvrMode = true;
    GetPrivateProfileString("MODE","FULLSCREEN_MODE",'\0', pathText, 1024, SettingPath);
    if(strcmp(pathText,"true") == 0)  fullscreenMode = true;


    mRoot = new Ogre::Root(mPluginsCfg);
    setupResources();
    bool carryOn = configure();
    if (!carryOn) return false;
    chooseSceneManager();

    if(oculusMode){
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media","FileSystem");
        // Load resources
        loadResources();
        
        if (!oculus.setupOgre(mSceneMgr, mWindow, mRoot))
        {
            Ogre::LogManager::getSingleton().logMessage(Ogre::LML_CRITICAL, "Failed to initialize Oculus Rift!");
            return false;
        }

        createCamera();
        createViewports();
    }
    else if (openvrMode) {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media", "FileSystem");
        loadResources();

        if (!openvr.initOpenVR(mSceneMgr, mWindow))
        {
            Ogre::LogManager::getSingleton().logMessage(Ogre::LML_CRITICAL, "Failed to initialize OpenVR!");
            return false;
        }

        createCamera();
        createViewports();
    }
    else{
        createCamera();
        createViewports();

        // Load resources
        loadResources();
    }
    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();

    // Create the scene
    createScene();

    createFrameListener();

	mSetupSuccessful = true;

	Ogre::LogManager::getSingleton().logMessage("Application Setup Complete!");
    return true;
};
//-------------------------------------------------------------------------------------
bool BaseApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    //static bool sended;
    if (mWindow->isClosed()) 
    {
        return false;
    }

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    return true;
}
//-------------------------------------------------------------------------------------
bool BaseApplication::keyPressed( const OIS::KeyEvent &arg )
{
    CEGUI::System &sys = CEGUI::System::getSingleton();
    
    sys.getDefaultGUIContext().injectKeyDown((CEGUI::Key::Scan)arg.key);
    sys.getDefaultGUIContext().injectChar(arg.text);
 
    if (arg.key == OIS::KC_LSHIFT || arg.key == OIS::KC_RSHIFT)
    {
        mShift = true;
    }
    else if (arg.key == OIS::KC_LCONTROL ||
             arg.key == OIS::KC_RCONTROL ||
             arg.key == OIS::KC_CAPITAL)
    {
        mCtrl = true;
    }
 
    return true;
}

bool BaseApplication::keyReleased( const OIS::KeyEvent &arg )
{
    CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp((CEGUI::Key::Scan)arg.key);

    if (arg.key == OIS::KC_LSHIFT || arg.key == OIS::KC_RSHIFT)
    {
        mShift = false;
    }
    else if (arg.key == OIS::KC_LCONTROL ||
             arg.key == OIS::KC_RCONTROL ||
             arg.key == OIS::KC_CAPITAL)
    {
        mCtrl = false;
    }
    else if (arg.key == OIS::KC_R)
    {
        if (oculusMode)
            oculus.resetOrientation();
        else if (openvrMode)
            openvr.resetOrientation();
    }

    return true;
}

CEGUI::MouseButton BaseApplication::convertButton(OIS::MouseButtonID buttonID)
{
    switch (buttonID)
    {
        case OIS::MB_Left:
        {
            return CEGUI::LeftButton;
            break;
        }
        case OIS::MB_Right:
        {
            return CEGUI::RightButton;
            break;
        }
        case OIS::MB_Middle:
        {
            return CEGUI::MiddleButton;
            break;
        }
        default:
        {
            return CEGUI::LeftButton;
            break;
        }
    }
}


bool BaseApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    //Stop preventing the automatic click.
    if (mWindowResized){ mWindowResized = false; }

    CEGUI::System::getSingleton().getDefaultGUIContext().injectMousePosition(arg.state.X.abs, arg.state.Y.abs);

    return true;
}

bool BaseApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    //Stop preventing the automatic click. And return.
    if (mWindowResized)
    {
        mWindowResized = false;
        return true;
    }

    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(convertButton(id));

    if (id == OIS::MB_Left)
    {
        mLMouseDown = true;
    }
    else if (id == OIS::MB_Right)
    {
        //CEGUI::MouseCursor::getSingleton().hide();
        mRMouseDown = true;
    }

    return true;
}

bool BaseApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(this->convertButton(id));
    
    // Left mouse button up
    if (id == OIS::MB_Left)
    {
        mLMouseDown = false;
    }
    // Right mouse button up
    else if (id == OIS::MB_Right)
    {
        //CEGUI::MouseCursor::getSingleton().show();
        mRMouseDown = false;
    }

    return true;
}

//Adjust mouse clipping area
void BaseApplication::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
    
    CEGUI::Sizef size(width, height);
    CEGUI::System::getSingleton().notifyDisplaySizeChanged(size);

    mWindowResized = true;
}

//Unattach OIS before window shutdown (very important under Linux)
void BaseApplication::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}
