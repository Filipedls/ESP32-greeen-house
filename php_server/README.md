# PHP temp log Server

## Deploying

### Before

Set the following env variable in the 'docker-compose.yml':
 - API_KEY=<your_api_key>
 - LOGIN_PASS=<your_login_pass>

### Deploying locally

Start:

	docker-compose up --build

Stop:

	docker-compose down -v

### Deploying on portainer

 * Build a new image, with the name 'esp32www:lastest';
 * Copy the docker-compose.yml to a new stack;
 * Replace the build arg in the www service by 'image: esp32www:lastest';

 /!\ Make sure all ports are available!
