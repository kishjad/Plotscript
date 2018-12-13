#include "output_widget.hpp"
#include <QString>
#include <string>
#include <QGraphicsTextItem>
#include <QDebug>

void OutputWidget::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);
	view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}


OutputWidget::OutputWidget(QWidget * parent):QWidget(parent) {
	setObjectName("output");
	scene = new QGraphicsScene();
	view = new QGraphicsView(scene);
	QVBoxLayout *outLayout = new QVBoxLayout();
	outLayout->addWidget(view);
	view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setLayout(outLayout);
	
}

void OutputWidget::showExpression(std::string output) {
	
	QGraphicsTextItem *expression = new QGraphicsTextItem();
	QString out = QString::fromStdString(output);
	expression->setPlainText(out);
	scene->addItem(expression);
	view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}


OutputWidget::~OutputWidget()
{
	delete scene;
	delete view;
	scene = nullptr;
	view = nullptr;
}

void OutputWidget::showCircle(double x, double y, double dia)
{
	QGraphicsEllipseItem *circle = new QGraphicsEllipseItem();
  QRectF rec(x - (dia / 2), y - (dia / 2), dia, dia);

	QPen *pen = new QPen;
	pen->setColor(QColor(Qt::black));
	pen->setWidth(0);
	circle->setPen(*pen);
	circle->setBrush(Qt::black);

	circle->setRect(rec);
	scene->addItem(circle);
	view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void OutputWidget::showLine(double x1, double y1, double x2, double y2, double thick)
{
	QGraphicsLineItem *line = new QGraphicsLineItem(x1, y1, x2, y2);
	QPen *pen = new QPen;
	pen->setWidth(thick);
	line->setPen(*pen);
	scene->addItem(line);
	view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void OutputWidget::showText(std::string input, double x, double y, double rotate, double scale)
{
	//removing the quotes
	input.erase(0, 1); // erase the first character
	input.erase(input.size() - 1); // erase the last character
	
	auto font = QFont("Monospace", scale);
	font.setStyleHint(QFont::TypeWriter);
	QGraphicsTextItem *expression = new QGraphicsTextItem();

	expression->setFont(font);

	expression->setPlainText(QString::fromStdString(input));
	
	expression->setTransformOriginPoint(expression->boundingRect().center());
	expression->setRotation(rotate * 180 / (std::atan2(0, -1)));
	expression->setX(x - expression->boundingRect().center().x());
	expression->setY(y - expression->boundingRect().center().y());

	scene->addItem(expression);
	view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void OutputWidget::showClear()
{
	scene->clear();
}

