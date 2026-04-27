# Instrucciones para Deploy en S3

## 1. Crear Bucket S3

1. Ir a AWS Console → S3
2. Click en "Create bucket"
3. Nombre: `mia-frontend-202404730` (debe ser único globalmente)
4. Region: Usar la misma que tu EC2
5. Desmarcar "Block all public access"
6. Click en "Create bucket"

## 2. Configurar Hosting Estático

1. Seleccionar el bucket creado
2. Ir a pestaña "Properties"
3. Scroll a "Static website hosting"
4. Click "Edit"
5. Habilitar "Static website hosting"
6. Index document: `index.html`
7. Error document: `index.html`
8. Click "Save changes"

## 3. Configurar Permisos

1. Ir a pestaña "Permissions"
2. Scroll a "Bucket policy"
3. Click "Edit"
4. Pegar siguiente policy:

```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Sid": "PublicReadGetObject",
            "Effect": "Allow",
            "Principal": "*",
            "Action": "s3:GetObject",
            "Resource": "arn:aws:s3:::mia-frontend-202404730/*"
        }
    ]
}