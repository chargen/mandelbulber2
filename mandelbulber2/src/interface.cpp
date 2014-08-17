/*
 * interface.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: krzysztof
 */


#include "interface.hpp"
#include "system.hpp"
#include <QTextStream>
#include <QtUiTools/QtUiTools>
#include "fractal_list.hpp"
#include <qpainter.h>
#include "render_job.hpp"
#include "calculate_distance.hpp"
#include "camera_target.hpp"
#include "error_message.hpp"
#include "my_ui_loader.h"
#include <QDial>
#include "render_ssao.h"

cInterface *mainInterface;

//constructor of interface (loading of ui files)
cInterface::cInterface()
{
	mainWindow = NULL;
	application = NULL;
	qimage = NULL;
	renderedImage = NULL;
	mainImage = NULL;
	progressBar = NULL;
	stopRequest = false;
	repeatRequest = false;
}

cInterface::~cInterface()
{
	if(renderedImage) delete renderedImage;
	if(progressBar) delete progressBar;
	if(qimage) delete qimage;
	if(mainImage) delete mainImage;
	if(mainWindow) delete mainWindow;
	if(application) delete application;
}

void cInterface::ShowUi(void)
{
	WriteLog("Prepare RenderWindow class");
	mainWindow = new RenderWindow;

	WriteLog("Restoring window geometry");
	mainWindow->restoreGeometry(mainWindow->settings.value("mainWindowGeometry").toByteArray());

	WriteLog("Restoring window state");
	mainWindow->restoreState(mainWindow->settings.value("mainWindowState").toByteArray());

	WriteLog("mainWindow->show()");
	mainWindow->show();

	WriteLog("Prepare RenderedImage class");
	renderedImage = new RenderedImage(mainWindow);
	mainWindow->ui->scrollAreaLayoutRenderedImage->addWidget(renderedImage);

	WriteLog("Prepare progress and status bar");
	progressBar = new QProgressBar(mainWindow->ui->statusbar);
	progressBar->setMaximum(1000);
	mainWindow->ui->statusbar->addPermanentWidget(progressBar);

	renderedImage->show();

	//loading default ui for all fractal components
	QString uiFilename = systemData.sharedDir + QDir::separator() + "qt" + QDir::separator() + "fractal_mandelbulb.ui";
	InitializeFractalUi(uiFilename);

	WriteLog("cInterface::ConnectSignals(void)");
	ConnectSignals();
	WriteLog("cInterface::ConnectSignals(void) finished");
}

void cInterface::ConnectSignals(void)
{
	QApplication::connect(mainWindow->ui->pushButton_render, SIGNAL(clicked()), mainWindow, SLOT(slotStartRender()));
	QApplication::connect(mainWindow->ui->pushButton_stop, SIGNAL(clicked()), mainWindow, SLOT(slotStopRender()));
	QApplication::connect(mainWindow->ui->button_selectBackgroundTexture, SIGNAL(clicked()), mainWindow, SLOT(slotSelectBackgroundTexture()));
	QApplication::connect(mainWindow->ui->text_file_background, SIGNAL(textChanged(const QString&)), mainWindow, SLOT(slotLineEditBackgroundTextureEdited(const QString&)));
	QApplication::connect(mainWindow->ui->button_selectEnvMapTexture, SIGNAL(clicked()), mainWindow, SLOT(slotSelectEnvMapTexture()));
	QApplication::connect(mainWindow->ui->text_file_envmap, SIGNAL(textChanged(const QString&)), mainWindow, SLOT(slotLineEditEnvMapTextureEdited(const QString&)));
	QApplication::connect(mainWindow->ui->button_selectLightMapTexture, SIGNAL(clicked()), mainWindow, SLOT(slotSelectLightMapTexture()));
	QApplication::connect(mainWindow->ui->text_file_lightmap, SIGNAL(textChanged(const QString&)), mainWindow, SLOT(slotLineEditLightMapTextureEdited(const QString&)));
	QApplication::connect(mainWindow->ui->comboBox_ambient_occlusion_mode, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedComboAmbientOcclussionMode(int)));
	QApplication::connect(mainWindow->ui->pushButton_apply_image_changes, SIGNAL(clicked()), mainWindow, SLOT(slotPressedImageApplyButton()));
	QApplication::connect(mainWindow->ui->comboBox_perspective_type, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedPerspectiveTypeCombo(int)));
	QApplication::connect(mainWindow->ui->spinbox_coloring_palette_offset, SIGNAL(valueChanged(double)), mainWindow, SLOT(slotSpinBoxPaletteOffsetChanged(double)));
	QApplication::connect(mainWindow->ui->pushButton_randomize, SIGNAL(clicked()), mainWindow, SLOT(slotRandomizeButtonPressed()));
	QApplication::connect(mainWindow->ui->pushButton_randomPalette, SIGNAL(clicked()), mainWindow, SLOT(slotNewRandomPalettePressed()));
	QApplication::connect(mainWindow->ui->spinboxInt_coloring_palette_size, SIGNAL(valueChanged(int)), mainWindow, SLOT(slotSpinBoxPaletteSizeChanged(int)));
	QApplication::connect(mainWindow->ui->pushButton_getPaletteFromImage, SIGNAL(clicked()), mainWindow, SLOT(slotGetPaletteFromImagePressed()));
	QApplication::connect(mainWindow->ui->button_calculateFog, SIGNAL(clicked()), mainWindow, SLOT(slotAutoFogPressed()));

	QApplication::connect(mainWindow->ui->comboBox_image_proportion, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedComboImageProportion(int)));
	QApplication::connect(mainWindow->ui->pushButton_resolution_preset_1080, SIGNAL(clicked()), mainWindow, SLOT(slotPressedResolutionPresset()));
	QApplication::connect(mainWindow->ui->pushButton_resolution_preset_1200, SIGNAL(clicked()), mainWindow, SLOT(slotPressedResolutionPresset()));
	QApplication::connect(mainWindow->ui->pushButton_resolution_preset_1440, SIGNAL(clicked()), mainWindow, SLOT(slotPressedResolutionPresset()));
	QApplication::connect(mainWindow->ui->pushButton_resolution_preset_2160, SIGNAL(clicked()), mainWindow, SLOT(slotPressedResolutionPresset()));
	QApplication::connect(mainWindow->ui->pushButton_resolution_preset_240, SIGNAL(clicked()), mainWindow, SLOT(slotPressedResolutionPresset()));
	QApplication::connect(mainWindow->ui->pushButton_resolution_preset_4320, SIGNAL(clicked()), mainWindow, SLOT(slotPressedResolutionPresset()));
	QApplication::connect(mainWindow->ui->pushButton_resolution_preset_480, SIGNAL(clicked()), mainWindow, SLOT(slotPressedResolutionPresset()));
	QApplication::connect(mainWindow->ui->pushButton_resolution_preset_600, SIGNAL(clicked()), mainWindow, SLOT(slotPressedResolutionPresset()));
	QApplication::connect(mainWindow->ui->pushButton_resolution_preset_720, SIGNAL(clicked()), mainWindow, SLOT(slotPressedResolutionPresset()));
	QApplication::connect(mainWindow->ui->spinboxInt_image_height, SIGNAL(valueChanged(int)), mainWindow, SLOT(slotImageHeightChanged(int)));

	QApplication::connect(mainWindow->ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
	QApplication::connect(mainWindow->ui->actionSave_docks_positions, SIGNAL(triggered()), mainWindow, SLOT(slotMenuSaveDocksPositions()));
	QApplication::connect(mainWindow->ui->actionSave_settings, SIGNAL(triggered()), mainWindow, SLOT(slotSaveSettings()));
	QApplication::connect(mainWindow->ui->actionLoad_settings, SIGNAL(triggered()), mainWindow, SLOT(slotLoadSettings()));
	QApplication::connect(mainWindow->ui->actionSave_as_JPG, SIGNAL(triggered()), mainWindow, SLOT(slotSaveImageJPEG()));
	QApplication::connect(mainWindow->ui->actionSave_as_PNG, SIGNAL(triggered()), mainWindow, SLOT(slotSaveImagePNG8()));
	QApplication::connect(mainWindow->ui->actionSave_as_PNG_16_bit, SIGNAL(triggered()), mainWindow, SLOT(slotSaveImagePNG16()));
	QApplication::connect(mainWindow->ui->actionSave_as_PNG_16_bit_with_alpha_channel, SIGNAL(triggered()), mainWindow, SLOT(slotSaveImagePNG16Alpha()));
	QApplication::connect(mainWindow->ui->actionAbout_Qt, SIGNAL(triggered()), mainWindow, SLOT(slotAboutQt()));
	QApplication::connect(mainWindow->ui->actionAbout_Mandelbulber, SIGNAL(triggered()), mainWindow, SLOT(slotAboutMandelbulber()));

	QApplication::connect(mainWindow->ui->comboBox_formula_1, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedFractalCombo(int)));
	QApplication::connect(mainWindow->ui->comboBox_formula_2, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedFractalCombo(int)));
	QApplication::connect(mainWindow->ui->comboBox_formula_3, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedFractalCombo(int)));
	QApplication::connect(mainWindow->ui->comboBox_formula_4, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedFractalCombo(int)));

	QApplication::connect(mainWindow->ui->scrollAreaForImage, SIGNAL(resized(int, int)), mainWindow, SLOT(slotImageScrolledAreaResized(int, int)));
	QApplication::connect(mainWindow->ui->comboBox_image_preview_scale, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedImageScale(int)));

	QApplication::connect(mainWindow->ui->checkBox_hybrid_fractal_enable, SIGNAL(stateChanged(int)), mainWindow, SLOT(slotCheckBoxHybridFractalChanged(int)));

	QApplication::connect(renderedImage, SIGNAL(mouseMoved(int, int)), mainWindow, SLOT(slotMouseMovedOnImage(int, int)));
	QApplication::connect(renderedImage, SIGNAL(singleClick(int, int, Qt::MouseButton)), mainWindow, SLOT(slotMouceClickOnImage(int, int, Qt::MouseButton)));

	// ------------ camera manipulation -----------
	QApplication::connect(mainWindow->ui->bu_move_up, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_down, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_left, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_right, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_forward, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_backward, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));

	QApplication::connect(mainWindow->ui->bu_rotate_up, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_down, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_left, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_right, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_roll_left, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_roll_right, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));

	QApplication::connect(mainWindow->ui->vect3_camera_x, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_y, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_z, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_target_x, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_target_y, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_target_z, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_rotation_x, SIGNAL(editingFinished()), mainWindow, SLOT(slotRotationEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_rotation_y, SIGNAL(editingFinished()), mainWindow, SLOT(slotRotationEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_rotation_z, SIGNAL(editingFinished()), mainWindow, SLOT(slotRotationEdited()));
	QApplication::connect(mainWindow->ui->logedit_camera_distance_to_target, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraDistanceEdited()));
	QApplication::connect(mainWindow->ui->logslider_camera_distance_to_target, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotCameraDistanceSlider(int)));
	QApplication::connect(mainWindow->ui->comboBox_camera_absolute_distance_mode, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotMovementStepModeChanged(int)));

	//------------------------------------------------

	ConnectSignalsForSlidersInWindow(mainWindow);
	MakeColorButtonsInWindow(mainWindow);
}

//Reading ad writing parameters from/to ui to/from parameters container
void cInterface::SynchronizeInterface(cParameterContainer *par, cParameterContainer *parFractal, enumReadWrite mode)
{
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_effects, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_effects, par, mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_image_adjustments, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_image_adjustments, par, mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_navigation, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_navigation, par, mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_rendering_engine, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_rendering_engine, par, mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->tabWidget_fractal_common, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->tabWidget_fractal_common, par, mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->tabWidget_fractal_hybrid, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->tabWidget_fractal_hybrid, par, mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->centralwidget, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->centralwidget, par, mode);

	WriteLog("SynchronizeInterfaceWindow(mainWindow->fractalWidgets[0], &parFractal[0], mode)");
	SynchronizeInterfaceWindow(mainWindow->fractalWidgets[0], &parFractal[0], mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->fractalWidgets[1], &parFractal[1], mode)");
	SynchronizeInterfaceWindow(mainWindow->fractalWidgets[1], &parFractal[1], mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->fractalWidgets[2], &parFractal[2], mode)");
	SynchronizeInterfaceWindow(mainWindow->fractalWidgets[2], &parFractal[2], mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->fractalWidgets[3], &parFractal[3], mode)");
	SynchronizeInterfaceWindow(mainWindow->fractalWidgets[3], &parFractal[3], mode);

	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->frame_iterations_formula_1, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->frame_iterations_formula_1, par, mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->frame_iterations_formula_2, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->frame_iterations_formula_2, par, mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->frame_iterations_formula_3, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->frame_iterations_formula_3, par, mode);
	WriteLog("SynchronizeInterfaceWindow(mainWindow->ui->frame_iterations_formula_4, par, mode)");
	SynchronizeInterfaceWindow(mainWindow->ui->frame_iterations_formula_4, par, mode);
}

//Reading ad writing parameters from/to selected widget to/from parameters container
void cInterface::SynchronizeInterfaceWindow(QWidget *window, cParameterContainer *par, enumReadWrite mode)
{
	QTextStream out(stdout);
	WriteLog("SynchronizeInterfaceWindow() started");

	//----------- QLineEdit -------------------
	{
		WriteLog("SynchronizeInterfaceWindow() QLineEdit");
		QList<QLineEdit *> widgetListLineEdit = window->findChildren<QLineEdit *>();
		QList<QLineEdit *>::iterator it;

		for (it = widgetListLineEdit.begin(); it != widgetListLineEdit.end(); ++it)
		{
			//out << "QLineEdit:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;

			QString name = (*it)->objectName();
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QLineEdit"))
			{
				QLineEdit *lineEdit = *it;
				QString text = lineEdit->text();
				//out << name << " - text: " << text << endl;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);
				//out << name << " - type: " << type << endl;

				//----- get vectors ------------
				if (type == QString("vect3"))
				{
					char lastChar = (parameterName.at(parameterName.length() - 1)).toLatin1();
					QString nameVect = parameterName.left(parameterName.length() - 2);

					if (mode == read)
					{
						double value = lineEdit->text().toDouble();
						//out << nameVect << " - " << lastChar << " axis = " << value << endl;
						CVector3 vect = par->Get<CVector3>(nameVect);

						switch (lastChar)
						{
							case 'x':
								vect.x = value;
								break;

							case 'y':
								vect.y = value;
								break;

							case 'z':
								vect.z = value;
								break;

							default:
								qWarning() << "cInterface::SynchronizeInterfaceWindow(): edit field " << nameVect << " has wrong axis name (is " << lastChar << ")" << endl;
								break;
						}
						par->Set(nameVect, vect);
					}
					else if (mode == write)
					{
						CVector3 vect = par->Get<CVector3>(nameVect);
						QString qtext;

						switch (lastChar)
						{
							case 'x':
								qtext = QString::number(vect.x, 'g', 20);
								break;

							case 'y':
								qtext = QString::number(vect.y, 'g', 20);
								break;

							case 'z':
								qtext = QString::number(vect.z, 'g', 20);
								break;

							default:
								qWarning() << "cInterface::SynchronizeInterfaceWindow(): edit field " << nameVect << " has wrong axis name (is " << lastChar << ")" << endl;
								break;
						}
						lineEdit->setText(qtext);
						lineEdit->setCursorPosition(0);
					}
				}

				//---------- get double scalars --------
				else if (type == QString("edit") || type == QString("logedit"))
				{
					if (mode == read)
					{
						double value = lineEdit->text().toDouble();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						double value = par->Get<double>(parameterName);
						lineEdit->setText(QString::number(value, 'g', 20));
						lineEdit->setCursorPosition(0);
					}
				}

				//----------- get texts ------------
				else if (type == QString("text"))
				{
					if (mode == read)
					{
						QString text = lineEdit->text();
						par->Set(parameterName, text);
					}
					else if (mode == write)
					{
						QString text = par->Get<QString>(parameterName);
						lineEdit->setText(text);
					}
				}
			}
		} //end foreach
	}

	//------------ Double spin-box --------------
	{
		WriteLog("SynchronizeInterfaceWindow() QDoubleSpinBox");
		QList<QDoubleSpinBox *> widgetListDoubleSpinBox = window->findChildren<QDoubleSpinBox*>();
		QList<QDoubleSpinBox *>::iterator it;
		for (it = widgetListDoubleSpinBox.begin(); it != widgetListDoubleSpinBox.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QDoubleSpinBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QDoubleSpinBox"))
			{
				QDoubleSpinBox *spinbox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("spinbox") || type == QString("spinboxd"))
				{
					if (mode == read)
					{
						double value = spinbox->value();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						double value = par->Get<double>(parameterName);
						spinbox->setValue(value);
					}
				}
				else if(type == QString("spinbox3") || type == QString("spinboxd3"))
				{
					char lastChar = (parameterName.at(parameterName.length() - 1)).toLatin1();
					QString nameVect = parameterName.left(parameterName.length() - 2);
					if (mode == read)
					{
						double value = spinbox->value();
						CVector3 vect = par->Get<CVector3>(nameVect);

						switch (lastChar)
						{
							case 'x':
								vect.x = value;
								break;

							case 'y':
								vect.y = value;
								break;

							case 'z':
								vect.z = value;
								break;

							default:
								qWarning() << "cInterface::SynchronizeInterfaceWindow(): " << type << " " << nameVect << " has wrong axis name (is " << lastChar << ")" << endl;
								break;
						}
						par->Set(nameVect, vect);
					}
					else if (mode == write)
					{
						CVector3 vect = par->Get<CVector3>(nameVect);
						double value = 0;

						switch (lastChar)
						{
							case 'x':
								value = vect.x;
								break;

							case 'y':
								value = vect.y;
								break;

							case 'z':
								value = vect.z;
								break;

							default:
								qWarning() << "cInterface::SynchronizeInterfaceWindow(): " << type << " " << nameVect << " has wrong axis name (is " << lastChar << ")" << endl;
								break;
						}
						spinbox->setValue(value);
					}
				}
			}
		}
	}

	//------------ integer spin-box --------------
	{
		WriteLog("SynchronizeInterfaceWindow() QSpinBox");
		QList<QSpinBox *> widgetListDoubleSpinBox = window->findChildren<QSpinBox*>();
		QList<QSpinBox *>::iterator it;
		for (it = widgetListDoubleSpinBox.begin(); it != widgetListDoubleSpinBox.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QDoubleSpinBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QSpinBox"))
			{
				QSpinBox *spinbox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("spinboxInt"))
				{
					if (mode == read)
					{
						int value = spinbox->value();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						int value = par->Get<int>(parameterName);
						spinbox->setValue(value);
					}
				}
			}
		}
	}

	//checkboxes
	{
		WriteLog("SynchronizeInterfaceWindow() QCheckBox");
		QList<QCheckBox *> widgetListDoubleSpinBox = window->findChildren<QCheckBox*>();
		QList<QCheckBox *>::iterator it;
		for (it = widgetListDoubleSpinBox.begin(); it != widgetListDoubleSpinBox.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QCheckBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QCheckBox"))
			{
				QCheckBox *checkbox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("checkBox"))
				{
					if (mode == read)
					{
						bool value = checkbox->isChecked();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						bool value = par->Get<bool>(parameterName);
						checkbox->setChecked(value);
					}
				}
			}
		}
	}

	//groupsBox with checkbox
	{
		WriteLog("SynchronizeInterfaceWindow() QGroupBox");
		QList<QGroupBox *> widgetListDoubleSpinBox = window->findChildren<QGroupBox*>();
		QList<QGroupBox *>::iterator it;
		for (it = widgetListDoubleSpinBox.begin(); it != widgetListDoubleSpinBox.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QGroupBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QGroupBox"))
			{
				QGroupBox *groupbox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("groupCheck"))
				{
					if (mode == read)
					{
						bool value = groupbox->isChecked();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						bool value = par->Get<bool>(parameterName);
						groupbox->setChecked(value);
					}
				}
			}
		}
	}

	//---------- color buttons -----------
	{
		WriteLog("SynchronizeInterfaceWindow() colorButtons");
		QList<QPushButton *> widgetListPushButton = window->findChildren<QPushButton*>();
		QList<QPushButton *>::iterator it;
		for (it = widgetListPushButton.begin(); it != widgetListPushButton.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QDoubleSpinBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QPushButton"))
			{
				QPushButton *colorButton = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("colorButton"))
				{
					if (mode == read)
					{
						sRGB color;
						color.R = colorButton->property("selectedColor_r").toInt();
						color.G = colorButton->property("selectedColor_g").toInt();
						color.B = colorButton->property("selectedColor_b").toInt();
						par->Set(parameterName, color);
					}
					else if (mode == write)
					{
						sRGB color = par->Get<sRGB>(parameterName);
						QColor qcolor(color.R / 256, color.G / 256, color.B / 256);
						MakeIconForButton(qcolor, colorButton);
						colorButton->setProperty("selectedColor_r", color.R);
						colorButton->setProperty("selectedColor_g", color.G);
						colorButton->setProperty("selectedColor_b", color.B);
					}
				}
			}
		}
	}
	WriteLog("SynchronizeInterfaceWindow() finished");

	//---------- colorpalette -----------
	{
		WriteLog("SynchronizeInterfaceWindow() colorpalette");
		QList<ColorPaletteWidget *> widgetListColorPalette = window->findChildren<ColorPaletteWidget*>();
		QList<ColorPaletteWidget *>::iterator it;
		for (it = widgetListColorPalette.begin(); it != widgetListColorPalette.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "ColorPalette:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("ColorPaletteWidget"))
			{
				ColorPaletteWidget *colorPaletteWidget = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("colorpalette"))
				{
					if (mode == read)
					{
						cColorPalette palette = colorPaletteWidget->GetPalette();
						par->Set(parameterName, palette);
					}
					else if (mode == write)
					{
						cColorPalette palette = par->Get<cColorPalette>(parameterName);
						colorPaletteWidget->SetPalette(palette);
					}
				}
			}
		}
	}

	//combo boxes
	{
		WriteLog("SynchronizeInterfaceWindow() combo boxes");
		QList<QComboBox *> widgetListPushButton = window->findChildren<QComboBox*>();
		QList<QComboBox *>::iterator it;
		for (it = widgetListPushButton.begin(); it != widgetListPushButton.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QComboBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QComboBox"))
			{
				QComboBox *comboBox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("comboBox"))
				{
					if (mode == read)
					{
						int selection = comboBox->currentIndex();

						if(parameterName.left(7) == QString("formula"))
						{
							selection = fractalList[selection].internalID;
						}
						par->Set(parameterName, selection);
					}
					else if (mode == write)
					{
						int selection = par->Get<int>(parameterName);
						if(parameterName.left(7) == QString("formula"))
						{
							for(int i=0; i<fractalList.size(); i++)
							{
								if(fractalList[i].internalID == selection)
								{
									selection = i;
									break;
								}
							}
						}
						comboBox->setCurrentIndex(selection);
					}
				}
			}
		}
	}
	WriteLog("SynchronizeInterfaceWindow() finished");
}

//automatic setting of event slots for all sliders
void cInterface::ConnectSignalsForSlidersInWindow(QWidget *window)
{
	WriteLog("ConnectSignalsForSlidersInWindow() started");

	QList<QSlider *> widgetList = window->findChildren<QSlider *>();
	QList<QSlider *>::iterator it;
	for (it = widgetList.begin(); it != widgetList.end(); ++it)
	{
		QString name = (*it)->objectName();
		if (name.length() > 1 && (*it)->metaObject()->className() == QString("QSlider"))
		{
			const QSlider *slider = *it;

			QString type, parameterName;
			GetNameAndType(name, &parameterName, &type);

			if (type == QString("slider"))
			{
				QApplication::connect(slider, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotSliderMoved(int)));

				QString spinBoxName = QString("spinbox_") + parameterName;
				QDoubleSpinBox *spinBox = slider->parent()->findChild<QDoubleSpinBox*>(spinBoxName);
				if (spinBox)
				{
					QApplication::connect(spinBox, SIGNAL(valueChanged(double)), mainWindow, SLOT(slotDoubleSpinBoxChanged(double)));
				}
				else
				{
					qWarning() << "ConnectSignalsForSlidersInWindow() error: spinbox " << spinBoxName << " doesn't exists" << endl;
				}
			}
			if (type == QString("logslider"))
			{
				QApplication::connect(slider, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotLogSliderMoved(int)));

				QString editFieldName = QString("logedit_") + parameterName;
				QLineEdit *lineEdit = slider->parent()->findChild<QLineEdit*>(editFieldName);
				if (lineEdit)
				{
					QApplication::connect(lineEdit, SIGNAL(textChanged(const QString&)), mainWindow, SLOT(slotLogLineEditChanged(const QString&)));
				}
				else
				{
					qWarning() << "ConnectSignalsForSlidersInWindow() error: lineEdit " << editFieldName << " doesn't exists" << endl;
				}
			}
			if (type == QString("sliderInt"))
			{
				QApplication::connect(slider, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotIntSliderMoved(int)));

				QString spinboxName = QString("spinboxInt_") + parameterName;
				QSpinBox *spinbox = slider->parent()->findChild<QSpinBox*>(spinboxName);
				if (spinbox)
				{
					QApplication::connect(spinbox, SIGNAL(valueChanged(int)), mainWindow, SLOT(slotIntSpinBoxChanged(int)));
				}
				else
				{
					qWarning() << "ConnectSignalsForSlidersInWindow() error: spinboxInt " << spinboxName << " doesn't exists" << endl;
				}
			}
			if (type == QString("slider3"))
			{
				QApplication::connect(slider, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotSlider3Moved(int)));
				QString spinboxName = QString("spinbox3_") + parameterName;
				QDoubleSpinBox *spinbox = slider->parent()->findChild<QDoubleSpinBox*>(spinboxName);
				if (spinbox)
				{
					QApplication::connect(spinbox, SIGNAL(valueChanged(double)), mainWindow, SLOT(slotSpinBox3Changed(double)));
				}
				else
				{
					qWarning() << "ConnectSignalsForSlidersInWindow() error: spinbox3 " << spinboxName << " doesn't exists" << endl;
				}
			}
		}
	}

	QList<QDial *> widgetList2 = window->findChildren<QDial *>(QString());
	QList<QDial *>::iterator it2;
	for (it2 = widgetList2.begin(); it2 != widgetList2.end(); ++it2)
	{
		QString name = (*it2)->objectName();
		if (name.length() > 1 && (*it2)->metaObject()->className() == QString("QDial"))
		{
			const QDial *dial = *it2;
			QString type, parameterName;
			GetNameAndType(name, &parameterName, &type);

			if (type == QString("dial3"))
			{
				QApplication::connect(dial, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotDial3Moved(int)));

				QString spinBoxName = QString("spinboxd3_") + parameterName;
				QDoubleSpinBox *spinBox = dial->parent()->findChild<QDoubleSpinBox*>(spinBoxName);
				if (spinBox)
				{
					QApplication::connect(spinBox, SIGNAL(valueChanged(double)), mainWindow, SLOT(slotSpinBoxD3Changed(double)));
				}
				else
				{
					qWarning() << "ConnectSignalsForSlidersInWindow() error: spinboxd3 " << spinBoxName << " doesn't exists" << endl;
				}
			}
			if (type == QString("dial"))
			{
				QApplication::connect(dial, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotDialMoved(int)));

				QString spinBoxName = QString("spinboxd_") + parameterName;
				QDoubleSpinBox *spinBox = dial->parent()->findChild<QDoubleSpinBox*>(spinBoxName);
				if (spinBox)
				{
					QApplication::connect(spinBox, SIGNAL(valueChanged(double)), mainWindow, SLOT(slotSpinBoxDChanged(double)));
				}
				else
				{
					qWarning() << "ConnectSignalsForSlidersInWindow() error: spinboxd " << spinBoxName << " doesn't exists" << endl;
				}
			}
		}
	}
	WriteLog("ConnectSignalsForSlidersInWindow() finished");
}

//automatic setting of event slots for all colorButtons and preparation of buttons
void cInterface::MakeColorButtonsInWindow(QWidget *window)
{
	WriteLog("MakeColorButtonsInWindow() started");

	QList<QPushButton *> widgetList = window->findChildren<QPushButton *>();
	QList<QPushButton *>::iterator it;
	for(it = widgetList.begin(); it != widgetList.end(); ++it)
	{
		QString name = (*it)->objectName();
		if(name.length() > 1 && (*it)->metaObject()->className() == QString("QPushButton"))
		{
			QPushButton *pushButton = (*it);
			QString type, parameterName;
			GetNameAndType(name, &parameterName, &type);

			if(type == QString("colorButton"))
			{
				pushButton->setText("");
				QColor color(255,255,255);
				MakeIconForButton(color, pushButton);
				QApplication::connect(pushButton, SIGNAL(clicked()), mainWindow, SLOT(slotPresedOnColorButton()));
			}
		}
	}
	WriteLog("MakeColorButtonsInWindow() finished");
}

//extract name and type string from widget name
void cInterface::GetNameAndType(QString name, QString *parameterName, QString *type)
{
	size_t firstDashPosition = name.indexOf("_");
	*type = name.left(firstDashPosition);
	*parameterName = name.mid(firstDashPosition + 1);
}

//initialize ui for hybrid fractal components
void cInterface::InitializeFractalUi(QString &uiFileName)
{
	WriteLog("cInterface::InitializeFractalUi(QString &uiFileName) started");
	MyUiLoader loader;

	QFile uiFile(uiFileName);

	if(uiFile.exists())
	{
		uiFile.open(QFile::ReadOnly);
		mainWindow->fractalWidgets[0] = loader.load(&uiFile);
		mainWindow->ui->verticalLayout_fractal_1->addWidget(mainWindow->fractalWidgets[0]);
		mainWindow->fractalWidgets[0]->show();

		mainWindow->fractalWidgets[1] = NULL;
		mainWindow->fractalWidgets[2] = NULL;
		mainWindow->fractalWidgets[3] = NULL;

		QStringList fractalNames;
		for (int i = 0; i < fractalList.size(); i++)
		{
			QString name = fractalList[i].nameInComboBox;
			fractalNames.append(name);
		}

		mainWindow->ui->comboBox_formula_1->clear();
		mainWindow->ui->comboBox_formula_1->addItems(fractalNames);
		mainWindow->ui->comboBox_formula_2->clear();
		mainWindow->ui->comboBox_formula_2->addItems(fractalNames);
		mainWindow->ui->comboBox_formula_3->clear();
		mainWindow->ui->comboBox_formula_3->addItems(fractalNames);
		mainWindow->ui->comboBox_formula_4->clear();
		mainWindow->ui->comboBox_formula_4->addItems(fractalNames);
	}
	else
	{
		cErrorMessage::showMessage(QString("Can't open file ") + uiFileName + QString(" Fractal ui files can't be loaded"), cErrorMessage::errorMessage, mainWindow);
	}
	WriteLog("cInterface::InitializeFractalUi(QString &uiFileName) finished");
}

void cInterface::StatusText(const QString &text, const QString &progressText, double progress)
{
	mainWindow->ui->statusbar->showMessage(text, 0);
	progressBar->setValue(progress * 1000.0);
	progressBar->setTextVisible(true);
	progressBar->setFormat(progressText);
}

double cInterface::ImageScaleComboSelection2Double(int index)
{
	double scales[] = {0.0, 4.0, 2.0, 1.0, 0.5, 0.25, 0.1};
	if(index < 7)
	{
		return scales[index];
	}
	else
	{
		qCritical() << "Wrong image scale";
		return -1.0;
	}
}

double cInterface::CalcMainImageScale(double scale, int previewWidth, int previewHeight, cImage *image)
{
	double scaleOut;
	if(scale == 0.0)
	{
		double scale1 = (double)previewHeight / image->GetHeight();
		double scale2 = (double)previewWidth / image->GetWidth();
		scaleOut = min(scale1, scale2);
	}
	else
	{
		scaleOut = scale;
	}
	return scaleOut;
}

void cInterface::StartRender(void)
{
	WriteLog("cInterface::StartRender(void)");
	if(mainImage->IsUsed())
	{
		stopRequest = true;
		repeatRequest = true;
		WriteLog("cInterface::StartRender(void): rendering terminate request");
	}
	else
	{
	  do
	  {
			repeatRequest = false;
	  	SynchronizeInterface(gPar, gParFractal, cInterface::read);
			cRenderJob *renderJob = new cRenderJob(gPar, gParFractal, mainImage, renderedImage);
			renderJob->Init(cRenderJob::still);
			renderJob->Execute();

			delete renderJob;
	  }
		while(repeatRequest);
	}
}

void cInterface::MoveCamera(QString buttonName)
{
	WriteLog("cInterface::MoveCamera(QString buttonName): button: " + buttonName);
	//get data from interface
	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);

	//get direction vector
	CVector3 direction;
	if(buttonName == "bu_move_left")
		direction = cameraTarget.GetRightVector() * (-1.0);
	else if(buttonName == "bu_move_right")
		direction = cameraTarget.GetRightVector() * ( 1.0);
	else if(buttonName == "bu_move_up")
		direction = cameraTarget.GetTopVector() * ( 1.0);
	else if(buttonName == "bu_move_down")
		direction = cameraTarget.GetTopVector() * (-1.0);
	else if(buttonName == "bu_move_forward")
		direction = cameraTarget.GetForwardVector() * (1.0);
	else if(buttonName == "bu_move_backward")
		direction = cameraTarget.GetForwardVector() * (-1.0);

	enumCameraMovementStepMode stepMode = (enumCameraMovementStepMode)gPar->Get<int>("camera_absolute_distance_mode");
	enumCameraMovementMode movementMode = (enumCameraMovementMode)gPar->Get<int>("camera_movement_mode");

	//movement step
	double step;
	if(stepMode == absolute)
	{
		step = gPar->Get<double>("camera_movenent_step");
	}
	else
	{
		double relativeStep = gPar->Get<double>("camera_movenent_step");

		CVector3 point;
		if(movementMode == moveTarget)
			point = target;
		else
			point = camera;

		double distance = GetDistanceForPoint(point, gPar, gParFractal);

		step = relativeStep * distance;
	}

	//movement
	if(movementMode == moveCamera)
		camera += direction * step;
	else if(movementMode == moveTarget)
		target += direction * step;
	else if (movementMode == fixedDistance)
	{
		camera += direction * step;
		target += direction * step;
	}

	//put data to interface
	gPar->Set("camera", camera);
	gPar->Set("target", target);

	//recalculation of camera-target
	cCameraTarget::enumRotationMode rollMode = 	(cCameraTarget::enumRotationMode)gPar->Get<int>("camera_straight_rotation");
	if(movementMode == moveCamera)
		cameraTarget.SetCamera(camera, rollMode);
	else if(movementMode == moveTarget)
		cameraTarget.SetTarget(target, rollMode);
	else if (movementMode == fixedDistance)
		cameraTarget.SetCameraTargetTop(camera, target, topVector);

	topVector = cameraTarget.GetTopVector();
	gPar->Set("camera_top", topVector);
	CVector3 rotation = cameraTarget.GetRotation();
	gPar->Set("camera_rotation", rotation * (180.0 / M_PI));
	double dist = cameraTarget.GetDistance();
	gPar->Set("camera_distance_to_target", dist);

	SynchronizeInterface(gPar, gParFractal, cInterface::write);

	StartRender();
}

void cInterface::CameraOrTargetEdited(void)
{
	WriteLog("cInterface::CameraOrTargetEdited(void)");

	//get data from interface before synchronization
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);

	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	camera = gPar->Get<CVector3>("camera");
	target = gPar->Get<CVector3>("target");

	//recalculation of camera-target
	cCameraTarget::enumRotationMode rollMode = 	(cCameraTarget::enumRotationMode)gPar->Get<int>("camera_straight_rotation");
	cameraTarget.SetCamera(camera, rollMode);
	cameraTarget.SetTarget(target, rollMode);

	topVector = cameraTarget.GetTopVector();
	gPar->Set("camera_top", topVector);
	CVector3 rotation = cameraTarget.GetRotation();
	gPar->Set("camera_rotation", rotation * (180.0 / M_PI));
	double dist = cameraTarget.GetDistance();
	gPar->Set("camera_distance_to_target", dist);

	SynchronizeInterface(gPar, gParFractal, cInterface::write);

}

void cInterface::RotateCamera(QString buttonName)
{
	WriteLog("cInterface::RotateCamera(QString buttonName): button: " + buttonName);

	//get data from interface
	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);
	double distance = cameraTarget.GetDistance();

	enumCameraRotationMode rotationMode = (enumCameraRotationMode)gPar->Get<int>("camera_rotation_mode");
	cCameraTarget::enumRotationMode rollMode = 	(cCameraTarget::enumRotationMode)gPar->Get<int>("camera_straight_rotation");

	CVector3 rotationAxis;
	if(rollMode == cCameraTarget::constantRoll)
	{
		if(buttonName == "bu_rotate_left")
			rotationAxis = CVector3(0.0, 0.0, 1.0);
		else if(buttonName == "bu_rotate_right")
			rotationAxis = CVector3(0.0, 0.0, -1.0);
		else if(buttonName == "bu_rotate_up")
		{
			rotationAxis = CVector3(1.0, 0.0, 0.0);
			rotationAxis = rotationAxis.RotateAroundVectorByAngle(CVector3(0.0, 0.0, 1.0), cameraTarget.GetRotation().x);
		}
		else if(buttonName == "bu_rotate_down")
		{
			rotationAxis = CVector3(-1.0, 0.0, 0.0);
			rotationAxis = rotationAxis.RotateAroundVectorByAngle(CVector3(0.0, 0.0, 1.0), cameraTarget.GetRotation().x);
		}
		else if(buttonName == "bu_rotate_roll_left")
			rotationAxis = cameraTarget.GetForwardVector() * (-1.0);
		else if(buttonName == "bu_rotate_roll_right")
			rotationAxis = cameraTarget.GetForwardVector() * (1.0);
	}
	else
	{
		if(buttonName == "bu_rotate_left")
			rotationAxis = cameraTarget.GetTopVector() * ( 1.0);
		else if(buttonName == "bu_rotate_right")
			rotationAxis = cameraTarget.GetTopVector() * (-1.0);
		else if(buttonName == "bu_rotate_up")
			rotationAxis = cameraTarget.GetRightVector() * (1.0);
		else if(buttonName == "bu_rotate_down")
			rotationAxis = cameraTarget.GetRightVector() * (-1.0);
		else if(buttonName == "bu_rotate_roll_left")
			rotationAxis = cameraTarget.GetForwardVector() * (-1.0);
		else if(buttonName == "bu_rotate_roll_right")
			rotationAxis = cameraTarget.GetForwardVector() * (1.0);
	}

	if(rotationMode == rotateAroundTarget)
		rotationAxis *= -1.0;

	//rotation of vectors
	CVector3 forwardVector = cameraTarget.GetForwardVector();
	double rotationStep = gPar->Get<double>("camera_rotation_step") * (M_PI / 180.0);
	forwardVector = forwardVector.RotateAroundVectorByAngle(rotationAxis, rotationStep);
	topVector = topVector.RotateAroundVectorByAngle(rotationAxis, rotationStep);

	if(rotationMode == rotateCamera)
	{
		target = camera + forwardVector * distance;
	}
	else
	{
		camera = target - forwardVector * distance;
	}

	//recalculation of camera-target
	cameraTarget.SetCameraTargetTop(camera, target, topVector);

	gPar->Set("camera", camera);
	gPar->Set("target", target);
	gPar->Set("camera_top", topVector);
	CVector3 rotation = cameraTarget.GetRotation();
	gPar->Set("camera_rotation", rotation * (180.0 / M_PI));
	double dist = cameraTarget.GetDistance();
	gPar->Set("camera_distance_to_target", dist);

	SynchronizeInterface(gPar, gParFractal, cInterface::write);

	StartRender();
}

void cInterface::RotationEdited(void)
{
	WriteLog("cInterface::RotationEdited(void)");
	//get data from interface before synchronization
	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);
	double distance = cameraTarget.GetDistance();
	CVector3 rotation = gPar->Get<CVector3>("camera_rotation") * (M_PI / 180.0);

	enumCameraRotationMode rotationMode = (enumCameraRotationMode)gPar->Get<int>("camera_rotation_mode");

	CVector3 forwardVector(0.0, 1.0, 0.0);
	forwardVector = forwardVector.RotateAroundVectorByAngle(CVector3(0.0, 1.0, 0.0), rotation.z);
	forwardVector = forwardVector.RotateAroundVectorByAngle(CVector3(1.0, 0.0, 0.0), rotation.y);
	forwardVector = forwardVector.RotateAroundVectorByAngle(CVector3(0.0, 0.0, 1.0), rotation.x);

	if(rotationMode == rotateCamera)
	{
		target = camera + forwardVector * distance;
	}
	else
	{
		camera = target - forwardVector * distance;
	}
	cameraTarget.SetCameraTargetRotation(camera, target, rotation.z);
	gPar->Set("camera", camera);
	gPar->Set("target", target);
	gPar->Set("camera_top", cameraTarget.GetTopVector());
	SynchronizeInterface(gPar, gParFractal, cInterface::write);
}

void cInterface::CameraDistanceEdited()
{
	WriteLog("cInterface::CameraDistanceEdited()");

	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);
	CVector3 forwardVector = cameraTarget.GetForwardVector();

	double distance = gPar->Get<double>("camera_distance_to_target");

	enumCameraMovementMode movementMode = (enumCameraMovementMode)gPar->Get<int>("camera_movement_mode");
	if(movementMode == moveTarget)
	{
		target = camera + forwardVector * distance;
	}
	else
	{
		camera = target - forwardVector * distance;
	}

	cameraTarget.SetCameraTargetTop(camera, target, topVector);
	gPar->Set("camera", camera);
	gPar->Set("target", target);
	gPar->Set("camera_top", cameraTarget.GetTopVector());
	SynchronizeInterface(gPar, gParFractal, cInterface::write);

}

void cInterface::IFSDefaultsDodecahedron(cParameterContainer *parFractal)
{
	double phi = (1 + sqrt(5.0))/2.0;
	parFractal->Set("IFS_scale", phi * phi);
	parFractal->Set("IFS_direction_0", CVector3(phi * phi, 1.0, -phi));
	parFractal->Set("IFS_direction_1", CVector3(-phi, phi * phi, 1.0));
	parFractal->Set("IFS_direction_2", CVector3(1.0, -phi, phi * phi));
	parFractal->Set("IFS_enabled_0", true);
	parFractal->Set("IFS_enabled_1", true);
	parFractal->Set("IFS_enabled_2", true);
	parFractal->Set("IFS_offset", CVector3(1.0, 1.0, 1.0));
	parFractal->Set("IFS_abs_x", true);
	parFractal->Set("IFS_abs_y", true);
	parFractal->Set("IFS_abs_z", true);
	parFractal->Set("IFS_menger_sponge_mode", false);
}

void cInterface::IFSDefaultsIcosahedron(cParameterContainer *parFractal)
{
	double phi = (1 + sqrt(5.0))/2.0;
	parFractal->Set("IFS_scale", 2.0);
	parFractal->Set("IFS_direction_3", CVector3(-phi * phi, 1.0, phi));
	parFractal->Set("IFS_direction_4", CVector3(phi, -phi * phi, 1.0));
	parFractal->Set("IFS_enabled_3", true);
	parFractal->Set("IFS_enabled_4", true);
	parFractal->Set("IFS_offset", CVector3(1.0, 0.0, phi));
	parFractal->Set("IFS_abs_x", true);
	parFractal->Set("IFS_abs_y", true);
	parFractal->Set("IFS_abs_z", true);
	parFractal->Set("IFS_menger_sponge_mode", false);
}

void cInterface::IFSDefaultsOctahedron(cParameterContainer *parFractal)
{
	parFractal->Set("IFS_scale", 2.0);
	parFractal->Set("IFS_direction_5", CVector3(1.0, -1.0, 0));
	parFractal->Set("IFS_direction_6", CVector3(1.0, 0.0, -1.0));
	parFractal->Set("IFS_direction_7", CVector3(0.0, 1.0, -1.0));
	parFractal->Set("IFS_enabled_5", true);
	parFractal->Set("IFS_enabled_6", true);
	parFractal->Set("IFS_enabled_7", true);
	parFractal->Set("IFS_offset", CVector3(1.0, 0.0, 0.0));
	parFractal->Set("IFS_abs_x", true);
	parFractal->Set("IFS_abs_y", true);
	parFractal->Set("IFS_abs_z", true);
	parFractal->Set("IFS_menger_sponge_mode", false);
}

void cInterface::IFSDefaultsMengerSponge(cParameterContainer *parFractal)
{
	parFractal->Set("IFS_scale", 3.0);
	parFractal->Set("IFS_direction_5", CVector3(1.0, -1.0, 0));
	parFractal->Set("IFS_direction_6", CVector3(1.0, 0.0, -1.0));
	parFractal->Set("IFS_direction_7", CVector3(0.0, 1.0, -1.0));
	parFractal->Set("IFS_enabled_5", true);
	parFractal->Set("IFS_enabled_6", true);
	parFractal->Set("IFS_enabled_7", true);
	parFractal->Set("IFS_offset", CVector3(1.0, 1.0, 1.0));
	parFractal->Set("IFS_abs_x", true);
	parFractal->Set("IFS_abs_y", true);
	parFractal->Set("IFS_abs_z", true);
	parFractal->Set("IFS_menger_sponge_mode", true);
}

void cInterface::IFSDefaultsReset(cParameterContainer *parFractal)
{
	for(int i=0; i<9; i++)
	{
		parFractal->Set("IFS_direction", i, CVector3(1.0, 0.0, 0.0));
		parFractal->Set("IFS_rotations", i, CVector3(0.0, 0.0, 0.0));
		parFractal->Set("IFS_distance", i, 0.0);
		parFractal->Set("IFS_enabled", i, false);
		parFractal->Set("IFS_intensity", i, 1.0);
	}
	parFractal->Set("IFS_offset", CVector3(1.0, 0.0, 0.0));
	parFractal->Set("IFS_abs_x", false);
	parFractal->Set("IFS_abs_y", false);
	parFractal->Set("IFS_abs_z", false);
	parFractal->Set("IFS_menger_sponge_mode", false);
	parFractal->Set("IFS_scale", 2.0);
	parFractal->Set("IFS_rotation", CVector3(0.0, 0.0, 0.0));
	parFractal->Set("IFS_rotation_enabled", false);
	parFractal->Set("IFS_edge", CVector3(0.0, 0.0, 0.0));
	parFractal->Set("IFS_edge_enabled", false);
}

void cInterface::ShowImageInLabel(QLabel *label, const QString &filename)
{
  QPixmap pixmap(filename);
  if(pixmap.isNull())
  {
  	label->setText("Error: Texture cannot be loaded");
  }
  else
  {
    pixmap = pixmap.scaledToWidth(label->width()*0.7, Qt::SmoothTransformation);
  	label->setPixmap(pixmap);
  }
}

void cInterface::RefreshMainImage()
{
	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	sImageAdjustments imageAdjustments;
	imageAdjustments.brightness = gPar->Get<double>("brightness");
	imageAdjustments.contrast = gPar->Get<double>("contrast");
	imageAdjustments.imageGamma = gPar->Get<double>("gamma");
	imageAdjustments.hdrEnabled = gPar->Get<bool>("hdr");
	mainImage->SetImageParameters(imageAdjustments);
	mainImage->CompileImage();

	if(gPar->Get<bool>("ambient_occlusion_enabled") && gPar->Get<int>("ambient_occlusion_mode") == params::AOmodeScreenSpace)
	{
		cParamRender params(gPar);
		sRenderData data;
		data.numberOfThreads = systemData.numberOfThreads;
		cRenderSSAO rendererSSAO(&params, &data, mainImage);
		rendererSSAO.RenderSSAO();
	}

	mainImage->ConvertTo8bit();
	mainImage->UpdatePreview();
	mainImage->GetImageWidget()->update();
}

cColorPalette cInterface::GetPaletteFromImage(const QString &filename)
{
	cColorPalette palette;
	QImage imagePalette(filename);

	SynchronizeInterfaceWindow(mainWindow->ui->groupCheck_fractal_color, gPar, cInterface::read);
	int paletteSize = gPar->Get<int>("coloring_palette_size");

	if(!imagePalette.isNull())
	{
		int width = imagePalette.width();
		int height = imagePalette.height();

		for (int i = 0; i < paletteSize; i++)
		{
			double angle = (double)i / paletteSize * M_PI * 2.0;
			double x = width / 2 + cos(angle) * width * 0.4;
			double y = height / 2 + sin(angle) * height * 0.4;
			QRgb pixel = imagePalette.pixel(x, y);
			sRGB pixelRGB(qRed(pixel), qGreen(pixel), qBlue(pixel));
			palette.AppendColor(pixelRGB);
		}

	}
	return palette;
}

void cInterface::AutoFog()
{
	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	double distance = GetDistanceForPoint(gPar->Get<CVector3>("camera"), gPar, gParFractal);
	double fogDensity = 0.5;
	double fogDistanceFactor = distance;
	double fogColour1Distance = distance * 0.5;
	double fogColour2Distance = distance;
	gPar->Set("volumetric_fog_distance_factor", fogDistanceFactor);
	gPar->Set("volumetric_fog_colour_1_distance", fogColour1Distance);
	gPar->Set("volumetric_fog_colour_2_distance", fogColour2Distance);
	gPar->Set("volumetric_fog_density", fogDensity);
	SynchronizeInterface(gPar, gParFractal, cInterface::write);
}

double cInterface::GetDistanceForPoint(CVector3 point, cParameterContainer *par, cParameterContainer *parFractal)
{
	cParamRender *params = new cParamRender(par);
	cFourFractals *fourFractals = new cFourFractals(parFractal, par);
	sDistanceIn in(point, 0, false);
	sDistanceOut out;
	double distance = CalculateDistance(*params, *fourFractals, in, &out);
	delete params;
	delete fourFractals;
	return distance;
}

double cInterface::GetDistanceForPoint(CVector3 point)
{
	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	double distance = GetDistanceForPoint(point, gPar, gParFractal);
	return distance;
}

void cInterface::MoveCameraByMouse(CVector2<double> screenPoint, Qt::MouseButton button)
{
	WriteLog("MoveCameraByMouse(CVector2<double> screenPoint, Qt::MouseButton button): button: " + button);
	//get data from interface
	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);

	enumCameraMovementStepMode stepMode = (enumCameraMovementStepMode)gPar->Get<int>("camera_absolute_distance_mode");
	enumCameraMovementMode movementMode = (enumCameraMovementMode)gPar->Get<int>("camera_movement_mode");
	RenderedImage::enumClickMode clickMode = (RenderedImage::enumClickMode)gPar->Get<int>("mouse_click_function");
	params::enumPerspectiveType perspType = (params::enumPerspectiveType)gPar->Get<int>("perspective_type");
	cCameraTarget::enumRotationMode rollMode = 	(cCameraTarget::enumRotationMode)gPar->Get<int>("camera_straight_rotation");
	double movementStep =  gPar->Get<double>("camera_movenent_step");
	double fov = gPar->Get<double>("fov");

	CVector2<double> imagePoint;
	imagePoint = screenPoint / mainImage->GetPreviewScale();

	int width = mainImage->GetWidth();
	int height = mainImage->GetHeight();

	if(imagePoint.x >= 0 && imagePoint.x < mainImage->GetWidth() && imagePoint.y >= 0 && imagePoint.y < mainImage->GetHeight())
	{
		double depth = mainImage->GetPixelZBuffer(imagePoint.x, imagePoint.y);

		CVector3 viewVector;
		double x2, z2;
		double aspectRatio = (double) width / height;

		CVector3 angles = cameraTarget.GetRotation();
		CRotationMatrix mRot;
		mRot.SetRotation(angles);

		switch(perspType)
		{
			case params::perspThreePoint:
			{
				x2 = ((double) imagePoint.x / width - 0.5) * aspectRatio;
				z2 = ((double) imagePoint.y / height - 0.5) * (-1.0);
				viewVector.x = x2 * fov;
				viewVector.y = 1.0;
				viewVector.z = z2 * fov;
				break;
			}

			case params::perspFishEye:
			case params::perspFishEyeCut:
			{
				x2 = M_PI * ((double) imagePoint.x / width - 0.5) * aspectRatio;
				z2 = M_PI * ((double) imagePoint.y / height - 0.5) * (-1.0);

				double r = sqrt(x2 * x2 + z2 * z2);

				if(r == 0)
				{
					viewVector.x = 0.0;
					viewVector.z = 0.0;
					viewVector.y = 1.0;
				}
				else
				{
					viewVector.x = x2 / r * sin(r * fov);
					viewVector.z = z2 / r * sin(r * fov);
					viewVector.y = cos(r * fov);
				}
				viewVector.Normalize();
				break;
			}

			case params::perspEquirectangular:
			{
				x2 = M_PI * ((double) imagePoint.x / width - 0.5) * aspectRatio; // --------- do sprawdzenia
				z2 = M_PI * ((double) imagePoint.y / height - 0.5) * (-1.0);
				viewVector.x = sin(fov * x2) * cos(fov * z2);
				viewVector.z = sin(fov * z2);
				viewVector.y = cos(fov * x2) * cos(fov * z2);
				viewVector.Normalize();
				break;
			}
		}

		viewVector = mRot.RotateVector(viewVector);

		CVector3 point = camera + viewVector * depth;

		switch(clickMode)
		{
			case RenderedImage::clickMoveCamera:
			{
				double distance = (camera - point).Length();
				double moveDistance = (stepMode == absolute) ? movementStep : distance * movementStep;
				if(stepMode == relative)
				{
					if(moveDistance > depth * 0.99) moveDistance = depth * 0.99;
				}

				if(button == Qt::RightButton)
				{
					moveDistance *= -1.0;
				}

				switch (movementMode)
				{
					case moveTarget:
						target = point;
						break;

					case moveCamera:
						camera += viewVector * moveDistance;
						break;

					case fixedDistance:
						camera += viewVector * moveDistance;
						target = point;
						break;
				}

				//recalculation of camera-target
				if(movementMode == moveCamera)
					cameraTarget.SetCamera(camera, rollMode);
				else if(movementMode == moveTarget)
					cameraTarget.SetTarget(target, rollMode);
				else if (movementMode == fixedDistance)
					cameraTarget.SetCameraTargetTop(camera, target, topVector);

				if(rollMode == cCameraTarget::constantRoll)
				{
					cameraTarget.SetCameraTargetRotation(camera, target, angles.z);
				}

				gPar->Set("camera", camera);
				gPar->Set("target", target);

				topVector = cameraTarget.GetTopVector();
				gPar->Set("camera_top", topVector);
				CVector3 rotation = cameraTarget.GetRotation();
				gPar->Set("camera_rotation", rotation * (180.0 / M_PI));
				double dist = cameraTarget.GetDistance();
				gPar->Set("camera_distance_to_target", dist);

				SynchronizeInterface(gPar, gParFractal, cInterface::write);

				renderedImage->setNewZ(depth - moveDistance);

				StartRender();

				break;
			}
			case RenderedImage::clickFogVisibility:
			{
				break;
			}
		}
	}
}

void cInterface::MovementStepModeChanged(int mode)
{
	SynchronizeInterface(gPar, gParFractal, cInterface::read);
	enumCameraMovementStepMode stepMode = (enumCameraMovementStepMode)mode;
	double distance = GetDistanceForPoint(gPar->Get<CVector3>("camera"), gPar, gParFractal);
	double oldStep = gPar->Get<double>("camera_movenent_step");
	double newStep;
	if(stepMode == absolute)
	{
		newStep = oldStep * distance;
	}
	else
	{
		newStep = oldStep / distance;
	}
	gPar->Set("camera_movenent_step", newStep);
	SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_navigation, gPar, cInterface::write);
}

//function to create icons with actual color in ColorButtons
void MakeIconForButton(QColor &color, QPushButton *pushbutton)
{
	const int w = 40;
	const int h = 15;
	QPixmap pix(w,h);
	QPainter painter(&pix);
	painter.fillRect(QRect(0,0,w,h), color);
	painter.drawRect(0,0,w-1,h-1);
	QIcon icon(pix);
	pushbutton->setIcon(icon);
	pushbutton->setIconSize(QSize(w,h));
}



