#ifndef INPUT_WIDGET_HPP
#define INPUT_WIDGET_HPP
#include <QWidget>
#include <QDebug>
#include <QPlainTextEdit>


class InputWidget : public QPlainTextEdit 
{
	Q_OBJECT
public:
	InputWidget(QWidget * parent = nullptr);
signals:
	void generatesignal(std::string);
protected:
	/*keyPressEvent checks for Shitt+Enter
	combination and then send the data to NotebookApp
	*/
	void keyPressEvent(QKeyEvent * e);

};
#endif