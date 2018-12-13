#ifndef OUTPUT_WIDGET_HPP
#define OUTPUT_WIDGET_HPP
#include "expression.hpp"
#include <QKeyEvent>
#include <QLayout>
#include <QWidget>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsView>

class OutputWidget : public QWidget {
	Q_OBJECT
public:
	OutputWidget(QWidget * parent = nullptr);
	QGraphicsScene *scene;
	QGraphicsView *view;
	~OutputWidget();

public slots:
	/*
	showExpression takes in the string to be displayed
	as the argument and uses QGraphicsTextItem to draw it*/
	void showExpression(std::string);

	/*
	showCircle takes in the center coordinates and the 
	diameter of the circle as the argument and uses 
	QGraphicsEllipseItem to draw it*/
	void showCircle(double x, double y, double dia);

	/*showLine takes in the coordinates of 2 end points of
	the line and also the thickness of the line and uses
	QGraphicsLineItem to draw it*/
	void showLine(double x1, double y1, double x2, double y2, double thick);

	/*showText takes in the string to be displayed
	as the argument and uses QGraphicsTextItem to draw it
	at coordinates (x,y)*/
	void showText(std::string input, double x, double y, double rotate, double size);

	/*showClear clears the scene*/
	void showClear();

private:
	void resizeEvent(QResizeEvent *event);

};
#endif