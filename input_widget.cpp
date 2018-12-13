#include "input_widget.hpp"	

InputWidget::InputWidget(QWidget * parent) : QPlainTextEdit(parent) {
	setObjectName("input");
}

void InputWidget::keyPressEvent(QKeyEvent * e) {

	if ((e->key() == Qt::Key_Return)  && (e->modifiers() == Qt::ShiftModifier)) {
		generatesignal(this->toPlainText().toStdString());
		this->clear();
	}
	else
	{
		QPlainTextEdit::keyPressEvent(e);
	}
}