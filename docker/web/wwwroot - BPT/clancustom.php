<?php
session_start();
require_once 'clancustomconfig.php';

// Logout
if (isset($_GET['sess']) && $_GET['sess'] == "logout") {
    session_destroy();
    header("location: clancustom.php");
    exit;
}

// Processar ações AJAX
if (isset($_POST['action']) && $_POST['action'] == 'update_image') {
    header('Content-Type: application/json');
    
    if (!isset($_SESSION["ID"])) {
        echo json_encode(array('success' => false, 'message' => 'Sessão expirada.'));
        exit;
    }
    
    $newImage = (int)$_POST['image_id'];
    $userId = $_SESSION["ID"];
    $leaderInfo = verificarLiderClan($userId);
    
    if (!$leaderInfo) {
        echo json_encode(array('success' => false, 'message' => 'Você não é líder!'));
        exit;
    }
    
    if (atualizarImagemClan($leaderInfo['clanName'], $newImage)) {
        echo json_encode(array('success' => true, 'message' => 'Imagem atualizada!', 'new_image' => $newImage));
    } else {
        echo json_encode(array('success' => false, 'message' => 'Erro ao atualizar.'));
    }
    exit;
}

if (isset($_POST['action']) && $_POST['action'] == 'update_motto') {
    header('Content-Type: application/json');
    
    if (!isset($_SESSION["ID"])) {
        echo json_encode(array('success' => false, 'message' => 'Sessão expirada.'));
        exit;
    }
    
    $newMotto = trim($_POST['motto']);
    $userId = $_SESSION["ID"];
    $leaderInfo = verificarLiderClan($userId);
    
    if (!$leaderInfo) {
        echo json_encode(array('success' => false, 'message' => 'Você não é líder!'));
        exit;
    }
    
    if (atualizarFraseClan($leaderInfo['clanName'], $newMotto)) {
        echo json_encode(array('success' => true, 'message' => 'Frase atualizada!', 'new_motto' => $newMotto));
    } else {
        echo json_encode(array('success' => false, 'message' => 'Erro ao atualizar.'));
    }
    exit;
}

// Processar login
if (isset($_POST['action']) && $_POST['action'] == "Logar") {
    $username = trim($_POST['username']);
    $password = trim($_POST['password']);

    if (empty($username) || empty($password)) {
        $error = 'Por favor, preencha todos os campos.';
    } else {
        $user = verificarLoginJogo($username, $password);
        
        if ($user) {
            $leaderInfo = verificarLiderClan($user['userId']);
            
            if ($leaderInfo) {
                $_SESSION["ID"] = $user['userId'];
                $_SESSION["clan_info"] = $leaderInfo;
                $_SESSION["full_name"] = $user['fullName'];
                header("location: clancustom.php");
                exit;
            } else {
                $error = 'Você não é líder de nenhum clan registrado!';
            }
        } else {
            $error = 'UserID ou Senha incorretos!';
        }
    }
}

// Se não está logado, mostrar login
if (!isset($_SESSION["ID"])) {
?>
<!DOCTYPE HTML>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Customização de Clan - Summit Tale</title>
<style>
body {
    font-family: Arial, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    margin: 0;
    padding: 20px;
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
}
.login-box {
    background: white;
    padding: 30px;
    border-radius: 10px;
    width: 100%;
    max-width: 400px;
    box-shadow: 0 5px 15px rgba(0,0,0,0.2);
}
.form-group {
    margin-bottom: 15px;
}
.form-group label {
    display: block;
    margin-bottom: 5px;
    font-weight: bold;
}
.form-group input {
    width: 100%;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 5px;
    box-sizing: border-box;
}
.btn {
    background: #667eea;
    color: white;
    padding: 12px 20px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    width: 100%;
    font-size: 16px;
}
.btn:hover {
    background: #5a6fd8;
}
.alert-error {
    background: #f8d7da;
    color: #721c24;
    border: 1px solid #f5c6cb;
    padding: 15px;
    border-radius: 5px;
    margin-bottom: 20px;
}
</style>
</head>
<body>
<div class="login-box">
    <h3 style="text-align: center; margin-bottom: 20px; color: #333;">🏰 Customização de Clan</h3>
    <p style="text-align: center; color: #666; margin-bottom: 20px;">
        Faça login com suas credenciais do jogo
    </p>
    
    <?php if (isset($error)): ?>
        <div class="alert-error"><?php echo htmlspecialchars($error); ?></div>
    <?php endif; ?>
    
    <form method="post" action="clancustom.php">
        <div class="form-group">
            <label>UserID: <span style="color: red;">*</span></label>
            <input name="username" type="text" required>
        </div>
        <div class="form-group">
            <label>Senha: <span style="color: red;">*</span></label>
            <input name="password" type="password" required>
        </div>
        <button type="submit" class="btn">🔑 ENTRAR</button>
        <input type="hidden" name="action" value="Logar">
    </form>
</div>
</body>
</html>
<?php
exit;
}

// Usuário logado - mostrar painel
$userId = $_SESSION["ID"];
$leaderInfo = verificarLiderClan($userId);

if (!$leaderInfo) {
    session_destroy();
    header("location: clancustom.php");
    exit;
}

$clanImages = obterImagensClan();
?>
<!DOCTYPE HTML>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Painel do Clan - Summit Tale</title>
<style>
body {
    font-family: Arial, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    margin: 0;
    padding: 20px;
    min-height: 100vh;
}
.container {
    max-width: 1200px;
    margin: 0 auto;
}
.panel {
    background: white;
    border-radius: 10px;
    padding: 30px;
    margin-bottom: 20px;
    box-shadow: 0 5px 15px rgba(0,0,0,0.1);
}
.clan-info {
    background: #f8f9fa;
    padding: 20px;
    border-radius: 8px;
    margin-bottom: 20px;
}
.current-image {
    max-width: 100px;
    border: 2px solid #ddd;
    border-radius: 5px;
}
.btn {
    background: #667eea;
    color: white;
    padding: 10px 20px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    margin-right: 10px;
    text-decoration: none;
    display: inline-block;
}
.btn:hover {
    background: #5a6fd8;
}
.btn-danger {
    background: #dc3545;
}
.btn-success {
    background: #28a745;
}
.images-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));
    gap: 15px;
    margin: 20px 0;
}
.image-item {
    text-align: center;
    cursor: pointer;
    padding: 10px;
    border: 2px solid transparent;
    border-radius: 8px;
    transition: all 0.3s;
}
.image-item:hover {
    border-color: #667eea;
    background: #f8f9fa;
}
.image-item.selected {
    border-color: #28a745;
    background: #d4edda;
}
.image-item img {
    width: 80px;
    height: 80px;
    border-radius: 5px;
}
.hidden { 
    display: none; 
}
.alert {
    padding: 15px;
    border-radius: 5px;
    margin-bottom: 20px;
}
.alert-success {
    background: #d4edda;
    color: #155724;
}
.alert-error {
    background: #f8d7da;
    color: #721c24;
}
.form-group {
    margin-bottom: 15px;
}
.form-group label {
    display: block;
    margin-bottom: 5px;
    font-weight: bold;
}
.form-group textarea {
    width: 100%;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 5px;
    box-sizing: border-box;
}
</style>
</head>
<body>
<div class="container">
    <div class="panel">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px;">
            <div>
                <h2>🏰 Painel do Clan: <?php echo htmlspecialchars($leaderInfo['clanName']); ?></h2>
                <p>Bem-vindo, <strong><?php echo htmlspecialchars($_SESSION["full_name"]); ?></strong>!</p>
            </div>
            <div>
                <a href="clancustom.php?sess=logout" class="btn btn-danger">🚪 Sair</a>
            </div>
        </div>
        
        <div id="messages"></div>
        
        <div class="clan-info">
            <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 20px; margin-bottom: 20px;">
                <div>
                    <h4>Tag/Imagem Atual</h4>
                    <p id="currentTag"><?php echo htmlspecialchars($leaderInfo['mIconCnt']); ?></p>
                    <img id="currentImage" src="<?php echo CLAN_IMAGES_URL . $leaderInfo['mIconCnt']; ?>.bmp" alt="Imagem atual" class="current-image">
                </div>
                <div>
                    <h4>Frase Atual</h4>
                    <p id="currentMotto"><?php echo htmlspecialchars($leaderInfo['note'] ?: 'Nenhuma frase definida'); ?></p>
                </div>
            </div>
            
            <div>
                <button class="btn" onclick="toggleImageGallery()">🏷️ Alterar Tag/Imagem</button>
                <button class="btn" onclick="toggleMottoForm()">📜 Alterar Frase</button>
            </div>
        </div>
        
        <!-- Galeria de Imagens -->
        <div id="imageGallery" class="hidden">
            <h4>Selecione uma nova imagem para o clan:</h4>
            <div class="images-grid" id="imagesGrid">
                <!-- Imagens serão carregadas aqui -->
            </div>
            <div style="margin-top: 20px;">
                <button class="btn btn-success" onclick="confirmImageSelection()">✅ Confirmar Seleção</button>
                <button class="btn" onclick="toggleImageGallery()">❌ Cancelar</button>
            </div>
        </div>
        
        <!-- Formulário de Frase -->
        <div id="mottoForm" class="hidden">
            <h4>Alterar Frase do Clan:</h4>
            <form id="mottoUpdateForm">
                <div class="form-group">
                    <label for="newMotto">Nova Frase (até 50 caracteres):</label>
                    <textarea id="newMotto" name="newMotto" maxlength="50" rows="3" placeholder="Digite a nova frase do clan..."><?php echo htmlspecialchars($leaderInfo['note']); ?></textarea>
                    <small>Caracteres restantes: <span id="charCount">50</span></small>
                </div>
                <div style="margin-top: 15px;">
                    <button type="submit" class="btn btn-success">✅ Salvar Frase</button>
                    <button type="button" class="btn" onclick="toggleMottoForm()">❌ Cancelar</button>
                </div>
            </form>
        </div>
    </div>
</div>

<script>
let selectedImage = null;
const clanImages = <?php echo json_encode($clanImages); ?>;

// Contador de caracteres
document.getElementById('newMotto').addEventListener('input', function() {
    const remaining = 50 - this.value.length;
    document.getElementById('charCount').textContent = remaining;
});

// Inicializar contador
document.getElementById('charCount').textContent = 50 - document.getElementById('newMotto').value.length;

// Manipular envio do formulário de frase
document.getElementById('mottoUpdateForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const newMotto = document.getElementById('newMotto').value;
    
    fetch('clancustom.php', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'action=update_motto&motto=' + encodeURIComponent(newMotto)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showMessage(data.message, 'success');
            document.getElementById('currentMotto').textContent = data.new_motto;
            toggleMottoForm();
        } else {
            showMessage(data.message, 'error');
        }
    });
});

function showMessage(message, type) {
    const messagesDiv = document.getElementById('messages');
    const alertClass = type === 'success' ? 'alert-success' : 'alert-error';
    messagesDiv.innerHTML = '<div class="alert ' + alertClass + '">' + message + '</div>';
    setTimeout(() => messagesDiv.innerHTML = '', 5000);
}

function toggleImageGallery() {
    const gallery = document.getElementById('imageGallery');
    const isHidden = gallery.classList.contains('hidden');
    
    if (isHidden) {
        gallery.classList.remove('hidden');
        loadImages();
    } else {
        gallery.classList.add('hidden');
    }
}

function toggleMottoForm() {
    const form = document.getElementById('mottoForm');
    form.classList.toggle('hidden');
}

function loadImages() {
    const grid = document.getElementById('imagesGrid');
    grid.innerHTML = '';
    
    clanImages.forEach(image => {
        const div = document.createElement('div');
        div.className = 'image-item';
        div.onclick = () => selectImage(image.value, div);
        
        div.innerHTML = '<img src="' + image.imageUrl + '" alt="' + image.label + '"><br><small>' + image.label + '</small>';
        grid.appendChild(div);
    });
}

function selectImage(imageId, element) {
    document.querySelectorAll('.image-item').forEach(item => item.classList.remove('selected'));
    element.classList.add('selected');
    selectedImage = imageId;
}

function confirmImageSelection() {
    if (!selectedImage) {
        showMessage('Por favor, selecione uma imagem primeiro.', 'error');
        return;
    }
    
    fetch('clancustom.php', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'action=update_image&image_id=' + selectedImage
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showMessage(data.message, 'success');
            document.getElementById('currentTag').textContent = data.new_image;
            document.getElementById('currentImage').src = '<?php echo CLAN_IMAGES_URL; ?>' + data.new_image + '.bmp';
            toggleImageGallery();
            selectedImage = null;
        } else {
            showMessage(data.message, 'error');
        }
    });
}
</script>
</body>
</html>