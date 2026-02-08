import asyncio
import asyncpg
import os
import hashlib

DB_URL = "postgresql://tess:tess@107.189.20.246:5662/tess"
ARTIFACTS_DIR = "release_artifacts"

def get_file_info(filename):
    """Calculates SHA256 and size of a file in release_artifacts."""
    filepath = os.path.join(ARTIFACTS_DIR, filename)
    if not os.path.exists(filepath):
        print(f"Warning: File {filename} not found locally. Using dummy values.")
        return "0000000000000000000000000000000000000000000000000000000000000000", 0
    
    sha256_hash = hashlib.sha256()
    with open(filepath, "rb") as f:
        # Read and update hash string value in blocks of 4K
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
            
    size = os.path.getsize(filepath)
    return sha256_hash.hexdigest(), size

async def insert_installer(conn, change_id, os_type, filename, url):
    # Calculate real hash and size
    shash, size = get_file_info(filename)
    
    # Check if exists
    row = await conn.fetchrow("SELECT id, shash FROM installers WHERE change_id = $1 AND os_type = $2", change_id, os_type)
    
    if row:
        print(f"Installer for {os_type} exists. Updating hash/size...")
        await conn.execute("""
            UPDATE installers 
            SET shash = $1, size_bytes = $2, url = $3, path = $4
            WHERE id = $5
        """, shash, size, url, filename, row['id'])
        print(f"Updated installer for {os_type}: {filename} (Size: {size}, Hash: {shash[:8]}...)")
    else:
        await conn.execute("""
            INSERT INTO installers (change_id, os_type, shash, url, path, size_bytes)
            VALUES ($1, $2, $3, $4, $5, $6)
        """, change_id, os_type, shash, url, filename, size)
        print(f"Inserted installer for {os_type}: {filename} (Size: {size}, Hash: {shash[:8]}...)")

async def main():
    print(f"Connecting to {DB_URL}...")
    try:
        conn = await asyncpg.connect(DB_URL)
    except Exception as e:
        print(f"Failed to connect to DB: {e}")
        return

    try:
        # 1. Ensure Tables Exist
        print("Ensuring tables exist...")
        await conn.execute("""
            CREATE TABLE IF NOT EXISTS releases (
                id SERIAL PRIMARY KEY,
                name VARCHAR(255) NOT NULL,
                version_number VARCHAR(50) NOT NULL,
                version_group VARCHAR(50),
                pushed_by VARCHAR(100),
                push_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                release_type VARCHAR(20) NOT NULL,
                status VARCHAR(20) CHECK (status IN ('draft', 'stable', 'deprecated')),
                note TEXT,
                UNIQUE (name, version_number)
            );
            
            CREATE TABLE IF NOT EXISTS sub_versions (
                id SERIAL PRIMARY KEY,
                release_id INT NOT NULL REFERENCES releases(id) ON DELETE CASCADE,
                version VARCHAR(50) NOT NULL,
                UNIQUE (release_id, version)
            );
            
            CREATE TABLE IF NOT EXISTS sub_changes (
                id SERIAL PRIMARY KEY,
                sub_version_id INT NOT NULL REFERENCES sub_versions(id) ON DELETE CASCADE,
                change_version VARCHAR(50) NOT NULL,
                note TEXT
            );
            
            CREATE TABLE IF NOT EXISTS installers (
                id SERIAL PRIMARY KEY,
                change_id INT NOT NULL REFERENCES sub_changes(id) ON DELETE CASCADE,
                os_type VARCHAR(20) NOT NULL CHECK (os_type IN ('linux', 'macos', 'windows')),
                shash VARCHAR(255) NOT NULL,
                url TEXT NOT NULL,
                path TEXT NOT NULL,
                size_bytes BIGINT,
                checksum_algo VARCHAR(20) DEFAULT 'sha256'
            );
        """)

        # 1.5 Try to update constraints
        try:
            await conn.execute("ALTER TABLE releases DROP CONSTRAINT IF EXISTS releases_release_type_check")
            await conn.execute("ALTER TABLE releases ADD CONSTRAINT releases_release_type_check CHECK (release_type IN ('Major', 'Minor', 'Bugfix', 'Security', 'End-of-life'))")
        except Exception as e:
            pass

        # 2. Insert Release 1.0.0
        print("Checking for Release 1.0.0...")
        release_id = await conn.fetchval("SELECT id FROM releases WHERE name = $1 AND version_number = $2", "Tess", "1.0.0")
        
        if not release_id:
            print("Creating Release 1.0.0...")
            release_id = await conn.fetchval("""
                INSERT INTO releases (name, version_number, version_group, pushed_by, release_type, status, note)
                VALUES ($1, $2, $3, $4, $5, $6, $7)
                RETURNING id
            """, "Tess", "1.0.0", "1.x", "sa1nt", "Major", "stable", "Initial Release")
            print(f"Created Release 1.0.0 with ID {release_id}")
        else:
            print(f"Release 1.0.0 already exists with ID {release_id}")

        # 3. Insert SubVersion
        sub_ver_id = await conn.fetchval("SELECT id FROM sub_versions WHERE release_id = $1 AND version = $2", release_id, "1.0.0")
        if not sub_ver_id:
            sub_ver_id = await conn.fetchval("""
                INSERT INTO sub_versions (release_id, version)
                VALUES ($1, $2)
                RETURNING id
            """, release_id, "1.0.0")
            print(f"Created SubVersion 1.0.0 with ID {sub_ver_id}")

        # 4. Insert SubChange
        sub_change_id = await conn.fetchval("SELECT id FROM sub_changes WHERE sub_version_id = $1 AND change_version = $2", sub_ver_id, "1.0.0")
        if not sub_change_id:
            sub_change_id = await conn.fetchval("""
                INSERT INTO sub_changes (sub_version_id, change_version, note)
                VALUES ($1, $2, $3)
                RETURNING id
            """, sub_ver_id, "1.0.0", "Initial release artifacts")
            print(f"Created SubChange 1.0.0 with ID {sub_change_id}")

        # 5. Insert Installers
        # Windows (MSI)
        await insert_installer(conn, sub_change_id, "windows", "TessInstaller.msi", "https://github.com/sa1nt/tess/releases/download/v1.0.0/TessInstaller.msi")
        
        # macOS (PKG)
        await insert_installer(conn, sub_change_id, "macos", "TessInstaller.pkg", "https://github.com/sa1nt/tess/releases/download/v1.0.0/TessInstaller.pkg")
        
        # Linux (TAR.GZ)
        await insert_installer(conn, sub_change_id, "linux", "tess-1.0.0.tar.gz", "https://github.com/sa1nt/tess/releases/download/v1.0.0/tess-1.0.0.tar.gz")

        print("Database population complete.")

    finally:
        await conn.close()

if __name__ == "__main__":
    asyncio.run(main())
