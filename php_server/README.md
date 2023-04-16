# PHP temp log Server

## Deploy locally

Services up:

	docker-compose up --build

Services down:

	docker-compose down -v

## How to deploy on portainer

 * Build a new image, with the name 'esp32www:lastest'
 * Copy the docker-compose.yml to a new stack
 * Replace the build arg in the www service by 'image: esp32www:lastest'
 * Make sure all ports are available!