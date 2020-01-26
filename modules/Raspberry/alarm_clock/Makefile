.PHONY : run
run : ./app.py setup
	./.venv/bin/python ./app.py


.PHONY : setup
setup : ./.venv/bin/activate
	python3 -m virtualenv -p /usr/bin/python3 .env
	./.venv/bin/python -m pip install -r requirements.txt